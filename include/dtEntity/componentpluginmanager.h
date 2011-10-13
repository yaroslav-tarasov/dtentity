#pragma once

/* -*-c++-*-
* dtEntity Game and Simulation Engine
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* Martin Scheffler
*/

#include <dtEntity/export.h>

#include <osg/ref_ptr>
#include <dtEntity/componentplugin.h>
#include <dtEntity/entityid.h>
#include <list>
#include <map>
#include <set>
#include <string>

/**
   Plugin access function:
   
   extern "C" MY_EXPORT_MACRO void CreatePluginFactories(std::list<dtEntity::ComponentPluginFactory*>& list)
   {
      list.push_back(new MyPluginFactory());
   }

*/

namespace dtEntity
{
   /**
    * The plugin manager is responsible for loading entity system factories from plugins,
    * keep them around until they are needed, and start and stop the entity systems.
    */
   class DT_ENTITY_EXPORT ComponentPluginManager
   {
      
   public:

      ComponentPluginManager(dtEntity::EntityManager& em);
      ~ComponentPluginManager();

      typedef std::map<ComponentType, osg::ref_ptr<ComponentPluginFactory> > PluginFactoryMap;
      typedef std::set<ComponentType> ActivePlugins;

      /**
       * is there a factory for an entity system of this type?
       */
      bool FactoryExists(ComponentType ctype);

      /**
       * load all libraries in dir and add component factories to registry
       */
      void LoadPluginsInDir(const std::string& path);

      /**
       * If entity of this type is registered, start it and add it to entity manager.
       * @param name Name of plugin to start
       * @return true if success
       */
      bool StartEntitySystem(ComponentType ctype);

      /**
       * stop all entity systems loaded from plugins and then unref the plugins
       */
      void UnloadAllPlugins();

      /**
       * Directly add a factory for starting an entity system (instead of loading
       * it from a plugin
       */
      void AddFactory(ComponentPluginFactory* factory);
      
      EntityManager& GetEntityManager() const { return *mEntityManager; }

      PluginFactoryMap& GetFactories() { return mFactories; }

   private:

      /** Get PluginFactory for Plugin with this name.
       * @param name The name of the Plugin/PluginFactory to get
       */
      ComponentPluginFactory* GetPluginFactory(ComponentType ctype);

      /** load all plugin factories from libraries found in path */
      void LoadPluginFactories(const std::string& baseLibName, std::list<osg::ref_ptr<ComponentPluginFactory> >& factories);

      /** map from plugin name -> plugin factory */
      PluginFactoryMap mFactories;

      dtEntity::EntityManager* mEntityManager;

   };
}
