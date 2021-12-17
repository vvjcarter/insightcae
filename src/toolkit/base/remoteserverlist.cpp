#include "remoteserverlist.h"

#include <cstdlib>
#include <regex>

#include "base/exception.h"
#include "base/tools.h"
#include "openfoam/openfoamcase.h"

#include "rapidxml/rapidxml_print.hpp"



using namespace std;
using namespace boost;




namespace insight {




RemoteServerList::RemoteServerList()
{
    SharedPathList paths;
    for ( const bfs_path& p: paths )
    {
        if ( exists(p) && is_directory ( p ) )
        {
            bfs_path serverlist = bfs_path(p) / "remoteservers.list";

            if ( exists(serverlist) )
            {

                // read xml
                string content;
                readFileIntoString(serverlist, content);

                using namespace rapidxml;
                xml_document<> doc;

                try {
                    doc.parse<0>(&content[0]);
                }
                catch (...)
                {
                    throw insight::Exception("Failed to parse XML from file "+serverlist.string());
                }

                auto *rootnode = doc.first_node("root");
                if (!rootnode)
                    throw insight::Exception("No valid \"remote\" node found in XML!");

                for (xml_node<> *e = rootnode->first_node(); e; e = e->next_sibling())
                {
                    if (e->name()==string("remoteServer"))
                    {
                        if ( auto rsc = RemoteServer::Config::create(e) )
                        {
                            std::string label = *rsc;

                            // replace entries, which were already existing:
                            // remove, if already present
                            auto i=findServerIterator(label);
                            if (i!=end()) erase(i);

                            insert(rsc);
                        }
                        else
                        {
                            std::string label("(unlabelled)");
                            if (auto *le=e->first_attribute("label"))
                                label=std::string(le->value());
                            insight::Warning("ignored invalid remote machine configuration: "+label);
                        }
                    }
                }

            }
        }
    }
}


RemoteServerList::RemoteServerList(const RemoteServerList& o)
  : std::set<std::shared_ptr<RemoteServer::Config> >(o)
{
}



filesystem::path RemoteServerList::firstWritableLocation() const
{
    insight::SharedPathList paths;
    for ( const bfs_path& p: paths )
    {
        insight::dbg()<<"checking, if "<<p.string()<<" is writable."<<std::endl;
        if ( insight::isInWritableDirectory(p) )
        {
            return p / "remoteservers.list";
        }
    }
    return bfs_path();
}




void RemoteServerList::writeConfiguration(const boost::filesystem::path& file)
{
  using namespace rapidxml;

  if (!boost::filesystem::exists(file.parent_path()))
      boost::filesystem::create_directories(file.parent_path());

  xml_document<> doc;
  xml_node<>* decl = doc.allocate_node(node_declaration);
  decl->append_attribute(doc.allocate_attribute("version", "1.0"));
  decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
  doc.append_node(decl);
  xml_node<> *rootnode = doc.allocate_node(node_element, "root");
  for (const auto& rs: *this)
  {
    xml_node<> *srvnode = doc.allocate_node(node_element, "remoteServer");
    rs->save(srvnode, doc);
    rootnode->append_node(srvnode);
  }

  doc.append_node(rootnode);

  ofstream f(file.string());
  f << doc;
}


RemoteServerList::iterator RemoteServerList::findServerIterator(
    const std::string& serverLabel ) const
{
  auto i = std::find_if(
        begin(), end(),
        [&](const value_type& entry)
        {
          return static_cast<std::string&>(*entry)==serverLabel;
        }
  );

  return i;
}


std::shared_ptr<RemoteServer::Config> RemoteServerList::findServer(
    const std::string& serverLabel ) const
{
  auto i = findServerIterator(serverLabel);
  if (i==end())
    {
      throw insight::Exception("Remote server \""+serverLabel+"\" not found in configuration!");
    }
  return *i;
}




RemoteServerList remoteServers;






} // namespace insight
