#include "sshlinuxserver.h"

#include <cstdlib>
#include <regex>

#include "base/exception.h"
#include "base/tools.h"
#include "openfoam/openfoamcase.h"

#include "rapidxml/rapidxml_print.hpp"

using namespace std;
using namespace boost;

namespace insight
{


SSHLinuxServer::Config::Config(
        const boost::filesystem::path& bp,
        const std::string hostName )
  : RemoteServer::Config(bp),
    hostName_(hostName)
{}

SSHLinuxServer::Config::Config(rapidxml::xml_node<> *e)
  :RemoteServer::Config(
     boost::filesystem::path(e->first_attribute("baseDirectory")->value())
     )
{
  auto* ha = e->first_attribute("host");
//  auto* lsaa = e->first_attribute("launchScript");
//  if (ha && lsaa)
//  {
//    throw insight::Exception("Invalid configuration of remote server "+label+": either host name or launch script must be specified!");
//  }
//  else if (ha)
  {
    hostName_=ha->value();
//    s.hasLaunchScript_=false;
//    (*this)[label]=s;
//    anything_read=true;
  }
//  else if (lsaa)
//  {
//    s.server_=lsaa->value();
//    s.hasLaunchScript_=true;
//    (*this)[label]=s;
//    anything_read=true;
//  }
}

std::shared_ptr<RemoteServer> SSHLinuxServer::Config::getInstanceIfRunning()
{
  auto srv = std::make_shared<SSHLinuxServer>( std::make_shared<Config>(*this) );
  if (srv->hostIsAvailable())
  {
    return srv;
  }
  else
    return nullptr;
}

std::shared_ptr<RemoteServer> SSHLinuxServer::Config::instance()
{
  auto srv = std::make_shared<SSHLinuxServer>( std::make_shared<Config>(*this) );
  if (!srv->hostIsAvailable())
    srv->launch();
  return srv;
}

bool SSHLinuxServer::Config::isDynamicallyAllocated() const
{
  return false;
}

void SSHLinuxServer::Config::save(rapidxml::xml_node<> *e, rapidxml::xml_document<>& doc) const
{
  e->append_attribute( doc.allocate_attribute( "label", this->c_str() ) );
  e->append_attribute( doc.allocate_attribute( "type", "SSHLinux" ) );
  e->append_attribute( doc.allocate_attribute( "host",
                                               hostName_.c_str() ) );
  e->append_attribute( doc.allocate_attribute( "baseDirectory", doc.allocate_string(
                                               defaultDirectory_.string().c_str() ) ) );
}









void SSHLinuxServer::runRsync
(
    const std::vector<std::string>& args,
    std::function<void(int,const std::string&)> pf
)
{
  assertRunning();

  RSyncProgressAnalyzer rpa;
  boost::process::child c
      (
       boost::process::search_path("rsync"),
       boost::process::args( args ),
       boost::process::std_out > rpa
      );
  rpa.runAndParse(c, pf);
}




SSHLinuxServer::SSHLinuxServer(ConfigPtr serverConfig)
{
  serverConfig_=serverConfig;
}




SSHLinuxServer::Config *SSHLinuxServer::serverConfig() const
{
  return std::dynamic_pointer_cast<Config>(serverConfig_).get();
}




string SSHLinuxServer::hostName() const
{
  return serverConfig()->hostName_;
}





std::pair<boost::filesystem::path,std::vector<std::string> >
SSHLinuxServer::commandAndArgs(const std::string& command) const
{
  std::string expr //= "bash -lc '"+command+"'";
          = command;

  SSHCommand ssh(hostName(), { expr });

  {
      auto& os = insight::dbg();
      os << ssh.command() << " ";
      for (const auto& a: ssh.arguments())
          os << " \"" + a + "\"";
      os << std::endl;
  }

  return { ssh.command(),
        ssh.arguments() };
}



SSHLinuxServer::BackgroundJob::BackgroundJob(RemoteServer &server, int remotePid)
  : RemoteServer::BackgroundJob(server),
    remotePid_(remotePid)
{}

void SSHLinuxServer::BackgroundJob::kill()
{
  server_.executeCommand(
        boost::str(boost::format
         ( "if ps -q %d >/dev/null; then kill %d; fi" )
          % remotePid_ % remotePid_
        ),
        true
        );
}


RemoteServer::BackgroundJobPtr SSHLinuxServer::launchBackgroundProcess(const std::string &cmd)
{
  boost::process::ipstream is;

  auto process = launchCommand(
        cmd+" & echo PID===$!===PID",
//#ifdef WIN32
        boost::process::std_out > is
//#else
//        boost::process::std_err > is
//#endif
        , boost::process::std_in < boost::process::null
        );

  if (!process->running())
  {
   throw insight::Exception("could not start background process");
  }

  boost::regex re("PID===([0-9]+)===PID");
  int remotePid = -1;
  int linesRead = 0;
  while (remotePid<0 && linesRead<100 && process->running())
  {
    std::string line;
    if (getline(is, line))
    {
      linesRead++;
      boost::smatch res;
      if (boost::regex_match(line, res, re))
      {
        remotePid=boost::lexical_cast<int>(res[1]);
        std::cout<<"remote process PID = "<<remotePid<<std::endl;
        break;
      }
    }
  }
  process->detach();
  return std::make_shared<BackgroundJob>(*this, remotePid);
}





void SSHLinuxServer::putFile
(
    const boost::filesystem::path& localFilePath,
    const boost::filesystem::path& remoteFilePath,
    std::function<void(int,const std::string&)> pf
    )
{
  CurrentExceptionContext ex("put file "+localFilePath.string()+" to remote location");
  assertRunning();

  std::vector<std::string> args=
      {
       localFilePath.string(),
       hostName()+":"+toUnixPath(remoteFilePath)
      };

  runRsync(args, pf);
}




void SSHLinuxServer::syncToRemote
(
    const boost::filesystem::path& localDir,
    const boost::filesystem::path& remoteDir,
    const std::vector<std::string>& exclude_pattern,
    std::function<void(int,const std::string&)> pf
)
{
  CurrentExceptionContext ex("upload local directory "+localDir.string()+" to remote location");
  assertRunning();

    std::vector<std::string> args=
        {
         "-az",
         "--delete",
         "--info=progress",

         "--exclude", "processor*",
         "--exclude", "*.foam",
         "--exclude", "postProcessing",
         "--exclude", "*.socket",
         "--exclude", "backup",
         "--exclude", "archive",
         "--exclude", "mnt_remote"
        };

    for (const auto& ex: exclude_pattern)
    {
      args.push_back("--exclude");
      args.push_back(ex);
    }

    args.push_back(localDir.string()+"/");
    args.push_back(hostName()+":"+toUnixPath(remoteDir));

    runRsync(args, pf);
}




void SSHLinuxServer::syncToLocal
(
    const boost::filesystem::path& localDir,
    const boost::filesystem::path& remoteDir,
    const std::vector<std::string>& exclude_pattern,
    std::function<void(int,const std::string&)> pf
)
{
  CurrentExceptionContext ex("download remote files to local directory");
  assertRunning();

  std::vector<std::string> args;

    args =
    {
      "-az",
      "--info=progress",
      "--exclude", "processor*",
      "--exclude", "*.foam",
      "--exclude", "*.socket",
      "--exclude", "backup",
      "--exclude", "archive",
      "--exclude", "mnt_remote"
    };

    for (const auto& ex: exclude_pattern)
    {
      args.push_back("--exclude");
      args.push_back(ex);
    }

    args.push_back(hostName()+":"+toUnixPath(remoteDir)+"/");
    args.push_back(localDir.string());

    runRsync(args, pf);
}





RemoteServer::PortMappingPtr SSHLinuxServer::makePortsAccessible(
    const std::set<int> &remoteListenerPorts,
    const std::set<int> &localListenerPorts)
{
  insight::CurrentExceptionContext ex("create port tunnels via SSH");

  return std::make_shared<SSHTunnelPortMapping>(
        *serverConfig(),
        remoteListenerPorts,
        localListenerPorts
        );
}




SSHLinuxServer::SSHTunnelPortMapping::SSHTunnelPortMapping(
    const Config& cfg,
    const std::set<int>& remoteListenerPorts,
    const std::set<int>& localListenerPorts
    )
{
  std::vector<std::string> args = { "-N" };

  for (const auto& rlp: remoteListenerPorts)
  {
    int localPort = findFreePort();
    insight::dbg()<<"remoteListenerPort: "<<rlp<<" / "<<localPort<<std::endl;
    remoteToLocal_.insert( std::pair<int,int>(rlp, localPort) );
    args.insert(
          std::end(args),
          {"-L", str(format("%d:%s:%d") % localPort % "127.0.0.1" % rlp) }
    );
  }

  for (const auto& llp: localListenerPorts)
  {
    int remotePort = findRemoteFreePort(cfg.hostName_);
    insight::dbg()<<"localListenerPorts: "<<llp<<" / "<<remotePort<<std::endl;
    localToRemote_.insert( std::pair<int,int>(llp, remotePort) );
    args.insert(
          std::end(args),
          {"-R", str(format("%d:%s:%d") % remotePort % "127.0.0.1" % llp ) }
    );
  }

//  args.push_back( server() );

  SSHCommand sc(cfg.hostName_, args);
  tunnelProcess_=
      boost::process::child
      (
       sc.command(), boost::process::args( sc.arguments() )
      )
   ;
}




SSHLinuxServer::SSHTunnelPortMapping::~SSHTunnelPortMapping()
{}




int SSHLinuxServer::SSHTunnelPortMapping::localListenerPort(int remoteListenerPort) const
{
  insight::CurrentExceptionContext ex(
        boost::str(boost::format("returning local port to remote listener port %d")%remoteListenerPort));
  return remoteToLocal_.at(remoteListenerPort);
}




int SSHLinuxServer::SSHTunnelPortMapping::remoteListenerPort(int localListenerPort) const
{
  insight::CurrentExceptionContext ex(
        boost::str(boost::format("returning remote port to local listener port %d")%localListenerPort));
  return localToRemote_.at(localListenerPort);
}



}
