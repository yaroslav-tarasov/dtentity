/*
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

#include "export.h"
#include <dtEntityWrappers/scriptcomponent.h>
#include <dtEntity/componentplugin.h>

namespace dtEntityWrappers
{


   ////////////////////////////////////////////////////////////////////////////////
   class DT_ENTITY_V8_EXPORT ScriptFactory : public dtEntity::ComponentPluginFactory
   {
   public:

      virtual bool Create(dtEntity::EntityManager* em, dtEntity::EntitySystem*& es)
      {
         es = new ScriptSystem(*em);
         return true;
      }

      /** get the name of the plugin */
      virtual std::string GetName() const
      {
         return "Script";
      }

      virtual dtEntity::ComponentType GetType() const
      {
         return ScriptSystem::TYPE;
      }

      /** get a description of the plugin */
      virtual std::string GetDescription() const
      {
         return "System for JavaScript/V8 scripts";
      }
   };
}


extern "C" DT_ENTITY_V8_EXPORT void CreatePluginFactories(std::list<dtEntity::ComponentPluginFactory*>& list)
{
   list.push_back(new dtEntityWrappers::ScriptFactory());
}