#include "wsllinuxserver.h"

#include <cstdlib>
#include <regex>

#include "base/exception.h"
#include "base/tools.h"
#include "openfoam/openfoamcase.h"

#include "rapidxml/rapidxml_print.hpp"


using namespace std;
using namespace boost;

namespace insight {




WSLLinuxServer::Config::Config(
        const boost::filesystem::path& bp,
        const std::string& distributionLabel )
  : RemoteServer::Config(bp),
    distributionLabel_(distributionLabel)
{}




WSLLinuxServer::Config::Config(rapidxml::xml_node<> *e)
  :RemoteServer::Config(
     boost::filesystem::path(e->first_attribute("baseDirectory")->value())
     )
{
  auto* ha = e->first_attribute("distributionLabel");

  distributionLabel_=ha->value();
}




std::shared_ptr<RemoteServer> WSLLinuxServer::Config::getInstanceIfRunning()
{
  return instance();
}




std::shared_ptr<RemoteServer> WSLLinuxServer::Config::instance()
{
  auto srv = std::make_shared<WSLLinuxServer>( std::make_shared<Config>(*this) );
  if (!srv->hostIsAvailable())
    throw insight::Exception("The WSL distribution "+distributionLabel_+" does not work!");
  return srv;
}




bool WSLLinuxServer::Config::isDynamicallyAllocated() const
{
  return false;
}




void WSLLinuxServer::Config::save(rapidxml::xml_node<> *e, rapidxml::xml_document<>& doc) const
{
  e->append_attribute( doc.allocate_attribute( "label", this->c_str() ) );
  e->append_attribute( doc.allocate_attribute( "type", "WSLLinux" ) );
  e->append_attribute( doc.allocate_attribute( "distributionLabel",doc.allocate_string(
                                               distributionLabel_.c_str() ) ) );
  e->append_attribute( doc.allocate_attribute( "baseDirectory", doc.allocate_string(
                                               defaultDirectory_.string().c_str() ) ) );
}




WSLLinuxServer::WSLLinuxServer(ConfigPtr serverConfig)
{
  serverConfig_=serverConfig;
}




WSLLinuxServer::Config *WSLLinuxServer::serverConfig() const
{
  return std::dynamic_pointer_cast<Config>(serverConfig_).get();
}





std::pair<boost::filesystem::path,std::vector<std::string> >
WSLLinuxServer::commandAndArgs(const std::string& command) const
{
  std::string windir("C:\\Windows");
  if (auto *wdv=getenv("WINDIR"))
  {
    windir=wdv;
  }
  auto wsl =boost::filesystem::path(windir+"\\Sysnative\\wsl.exe"); // weird: system32 does not work...

  insight::dbg()<<"WSL executable: "<<wsl<<std::endl;
  return {
    wsl,
    { "-d", serverConfig()->distributionLabel_, "--", "bash", "-lc", command }
  };
}




WSLLinuxServer::BackgroundJob::BackgroundJob(
    RemoteServer &server,
    std::unique_ptr<boost::process::child> process )
  : RemoteServer::BackgroundJob(server),
    process_(std::move(process))
{}




void WSLLinuxServer::BackgroundJob::kill()
{
  if (process_)
  {
    process_->terminate();
    process_->wait();
    process_.reset();
  }
}




RemoteServer::BackgroundJobPtr WSLLinuxServer::launchBackgroundProcess(const std::string &cmd)
{
  auto process = launchCommand(cmd);

  if (!process->running())
  {
   throw insight::Exception("could not start background process");
  }

  return std::make_shared<BackgroundJob>(*this, std::move(process));
}




void WSLLinuxServer::runRsync
(
    const std::vector<std::string>& args,
    std::function<void(int,const std::string&)> pf
)
{
  CurrentExceptionContext ex("runRsync, args: "+boost::join(args, " "));

  assertRunning();

  boost::process::ipstream is;

  std::string joinedArgs="rsync";
  for (const auto& a: args)
  {
    joinedArgs+=" "+escapeShellSymbols(a);
  }
  auto ca = commandAndArgs(joinedArgs);

  insight::dbg() << ca.first << " " << boost::join(ca.second, " ") << std::endl;

  RSyncProgressAnalyzer rpa;
  boost::process::child c
  (
   ca.first, boost::process::args(ca.second),
   boost::process::std_out > rpa,
   boost::process::std_err > stderr,
   boost::process::std_in < boost::process::null
  );
  rpa.runAndParse(c, pf);
}




boost::filesystem::path convertWindowsPathToWSLPath(const boost::filesystem::path& wp)
{
  auto wpg = boost::filesystem::absolute(wp).generic_path();
  auto drive = wpg.root_name().string().substr(0, 1);
  boost::algorithm::to_lower(drive);
  return (boost::filesystem::path("/mnt") / drive / wpg.relative_path()).generic_path();
}




void WSLLinuxServer::putFile
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
       convertWindowsPathToWSLPath(localFilePath).string(),
       remoteFilePath.string()
      };

  runRsync(args, pf);
}




void WSLLinuxServer::syncToRemote
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

    args.push_back(convertWindowsPathToWSLPath(localDir).string()+"/");
    args.push_back(remoteDir.string());

    runRsync(args, pf);
}




void WSLLinuxServer::syncToLocal
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

    args.push_back(remoteDir.string()+"/");
    args.push_back(convertWindowsPathToWSLPath(localDir).string());

    runRsync(args, pf);
}


std::mutex g_child_mutex;

ToolkitVersion WSLLinuxServer::checkInstalledVersion()
{
  boost::process::ipstream out;

  std::string packageName="insightcae";

  if (ToolkitVersion::current().branch()=="next-release" || ToolkitVersion::current().branch()=="master")
  {
    packageName="insightcae-ce";
  }

  std::string cmd="LC_ALL=C sudo apt policy "+packageName;

  int ret = executeCommand(
      cmd, false,
      boost::process::std_out > out,
      boost::process::std_err > stderr,
      boost::process::std_in < boost::process::null
      );


  dbg()<<"ret="<<ret<<std::endl;
  if (ret==0)
  {
    boost::regex pat("^  Installed: (.*)\\.(.*)\\.(.*)-(.*)~(.*)$");
    std::string line;
    while (getline(out, line))
    {
      dbg()<<line<<std::endl;
      boost::smatch m;
      if (boost::regex_search(line, m, pat))
      {
        dbg()<<m[1]<<" "<<m[2]<<" "<<m[3]<<" "<<m[4]<<" "<<m[5]<<std::endl;
        int majorVersion = lexical_cast<int>(m[1]);
        int minorVersion = lexical_cast<int>(m[2]);
        std::string patchVersion = m[3]+"-"+m[4];
        return ToolkitVersion(majorVersion, minorVersion, patchVersion, "");
      }
    }
    throw insight::Exception("Could not parse version information.\nMaybe package is not installed?");
  }
  else
  {
    throw insight::Exception("Could not execute command for version check (\""+cmd+"\")");
  }
}




void WSLLinuxServer::updateInstallation()
{
  std::vector<std::string> cmds={
    "sudo apt update -y",
    "sudo apt install -y insightcae"
  };

  for (const auto& cmd: cmds)
  {
    int ret = executeCommand(
          cmd, false,
          boost::process::std_out > stdout,
          boost::process::std_err > stderr,
          boost::process::std_in < boost::process::null
          );

    if (ret!=0)
    {
      throw insight::Exception("Could not execute WSL update command (\""+cmd+"\")");
    }
  }
}




} // namespace insight
