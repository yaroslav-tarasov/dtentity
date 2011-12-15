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

#include <dtEntity/mapcomponent.h>

#include <dtEntity/basemessages.h>
//#include <dtEntity/xercesmapencoder.h>
#include <dtEntity/rapidxmlmapencoder.h>
#include <assert.h>
#include <dtEntity/spawner.h>
#include <sstream>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <OpenThreads/ScopedLock>
#include <fstream>


#ifdef WIN32
   #include <Rpc.h>
   #include <Rpcdce.h>
#else
#include <uuid/uuid.h>
#endif

namespace dtEntity
{
   const StringId MapComponent::TYPE(SID("Map"));   
   const StringId MapComponent::EntityNameId(SID("EntityName"));  
   const StringId MapComponent::MapNameId(SID("MapName"));  
   const StringId MapComponent::SpawnerNameId(SID("SpawnerName"));  
   const StringId MapComponent::UniqueIdId(SID("UniqueId"));  
   const StringId MapComponent::SaveWithMapId(SID("SaveWithMap"));
   const StringId MapComponent::VisibleInEntityListId(SID("VisibleInEntityList"));
   
   
   ////////////////////////////////////////////////////////////////////////////
   MapComponent::MapComponent()
      : mSpawnerNameProp(
           DynamicStringProperty::SetValueCB(this, &MapComponent::SetSpawnerName),
           DynamicStringProperty::GetValueCB(this, &MapComponent::GetSpawnerName)
        )
      , mSpawner(NULL)
      , mOwner(NULL)
   {
      Register(EntityNameId, &mEntityName);
      Register(MapNameId, &mMapName);
      Register(SpawnerNameId, &mSpawnerNameProp);
      Register(UniqueIdId, &mUniqueId);
      Register(SaveWithMapId, &mSaveWithMap);
      Register(VisibleInEntityListId, &mVisibleInEntityList);
      mSaveWithMap.Set(true);
      mVisibleInEntityList.Set(true);

#ifdef WIN32
   GUID guid;
   
   if( UuidCreate( &guid ) == RPC_S_OK )
   {
      unsigned char* guidChar;

      if( UuidToString( const_cast<UUID*>(&guid), &guidChar ) == RPC_S_OK )
      {
         mUniqueId.Set(reinterpret_cast<const char*>(guidChar) );
         if(RpcStringFree(&guidChar) != RPC_S_OK) 
         {
            LOG_ERROR("Could not free memory.");
         }
      }
      else
      {
         LOG_WARNING("Could not convert UniqueId to std::string." );
      }
   }
   else
   {
      LOG_WARNING("Could not generate UniqueId." );
   }
#else
   uuid_t uuid;
   uuid_generate( uuid );

   char buffer[37];
   uuid_unparse(uuid, buffer);

   mUniqueId.Set(buffer);
#endif
   }
    
   ////////////////////////////////////////////////////////////////////////////
   MapComponent::~MapComponent()
   {
   }

   ////////////////////////////////////////////////////////////////////////////
   void MapComponent::OnPropertyChanged(StringId propname, Property& prop)
   {
      if(propname == UniqueIdId)
      {
         SetUniqueId(mUniqueId.Get());
      }
   }

   ////////////////////////////////////////////////////////////////////////////
   std::string MapComponent::GetSpawnerName() const
   {
      return mSpawner == NULL ? "" : mSpawner->GetName();
   }

   ////////////////////////////////////////////////////////////////////////////
   void MapComponent::SetSpawnerName(const std::string& name)
   {
      if(name == "")
      {
         mSpawner = NULL;
      }
      else
      {
         MapSystem* ms;
         mOwner->GetEntityManager().GetEntitySystem(TYPE, ms);
         ms->GetSpawner(name, mSpawner);
      }
   }

   ////////////////////////////////////////////////////////////////////////////
   void MapComponent::SetUniqueId(const std::string& v) { 
      mUniqueId.Set(v);
      if(mOwner != NULL)
      {
         MapSystem* ms;
         mOwner->GetEntityManager().GetEntitySystem(TYPE, ms);
         ms->OnEntityChangedUniqueId(mOwner->GetId(), v);
      }
   }

   ////////////////////////////////////////////////////////////////////////////
   MapSystem::MapSystem(EntityManager& em)
      : DefaultEntitySystem<MapComponent>(em)
      , mPluginManager(em, mMessageFactory)
      , mCurrentScene("")
   {

      mSpawnEntityFunctor = MessageFunctor(this, &MapSystem::OnSpawnEntity);
      em.RegisterForMessages(SpawnEntityMessage::TYPE, mSpawnEntityFunctor, "MapSystem::OnSpawnEntity");
      mDeleteEntityFunctor = MessageFunctor(this, &MapSystem::OnDeleteEntity);
      em.RegisterForMessages(DeleteEntityMessage::TYPE, mDeleteEntityFunctor, "MapSystem::OnDeleteEntity");

      RegisterCommandMessages(mMessageFactory);
      RegisterSystemMessages(mMessageFactory);
   }

   ////////////////////////////////////////////////////////////////////////////
   MapSystem::~MapSystem()
   {
   }

   ////////////////////////////////////////////////////////////////////////////
   void MapSystem::OnAddedToEntityManager(dtEntity::EntityManager& em)
   {
      //mMapEncoder = new XercesMapEncoder(em);
      mMapEncoder = new RapidXMLMapEncoder(em);
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::CreateComponent(EntityId eid, Component*& component)
   {
      bool ret = BaseClass::CreateComponent(eid, component);
      if(ret)
      {
         std::string uid = component->GetString(MapComponent::UniqueIdId);
         if(uid != "")
         {
            if(mEntitiesByUniqueId.find(uid) != mEntitiesByUniqueId.end())
            {
               LOG_ERROR("Entity with this name already exists!");
               DeleteComponent(eid);
               return false;
            }

            mEntitiesByUniqueId[uid] = eid;
         }
      }
      return ret;
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::DeleteComponent(EntityId eid)
   {
      ComponentStore::iterator i = mComponents.find(eid);
      if(i != mComponents.end())
      {
         mEntitiesByUniqueId.erase(i->second->GetString(MapComponent::UniqueIdId));
      }
      return BaseClass::DeleteComponent(eid);
   }

   ////////////////////////////////////////////////////////////////////////////
   void MapSystem::OnEntityChangedUniqueId(EntityId id, const std::string& newUniqueId)
   {
      typedef std::map<std::string, EntityId> UIMap;
      MapComponent* comp;
      bool success = GetEntityManager().GetComponent(id, comp);
      if(!success)
      {
         LOG_ERROR("HUCH?");
         return;
      }

      std::string oldUniqueId = comp->GetUniqueId();

      UIMap::iterator i = mEntitiesByUniqueId.find(oldUniqueId);
      if(i != mEntitiesByUniqueId.end())
      {
         if(oldUniqueId == newUniqueId)
         {
            return;
         }
         mEntitiesByUniqueId.erase(i);
      }

      UIMap::iterator j = mEntitiesByUniqueId.find(newUniqueId);
      if(j != mEntitiesByUniqueId.end())
      {
         LOG_ERROR("An entity with this unique id already exists!");
      }  
      else
      {
         mEntitiesByUniqueId[newUniqueId] = id;
      }
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::LoadScene(const std::string& path)
   {
      bool success = mMapEncoder->LoadSceneFromFile(path);
      
      SceneLoadedMessage msg;
      msg.SetSceneName(path);
      GetEntityManager().EmitMessage(msg);
      mCurrentScene = path;
      return success;
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::UnloadScene()
   {
      SceneUnloadedMessage msg;
      GetEntityManager().EmitMessage(msg);

      // copy map name list so that removing maps from original list does not
      // cause crash
      std::set<std::string> maps = mLoadedMaps;
      for(std::set<std::string>::iterator i = maps.begin(); i != maps.end(); ++i)
      {
         UnloadMap(*i);
      }
      mCurrentScene = "";
      return true;
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::SaveScene(const std::string& path, bool saveAllMaps)
   {
     bool success = mMapEncoder->SaveSceneToFile(path);

      if(success && saveAllMaps)
      {
         std::set<std::string>::const_iterator i;
         for(i = mLoadedMaps.begin(); i != mLoadedMaps.end(); ++i)
         {
            bool success = SaveMap(*i);
            if(!success)
            {
               LOG_ERROR("Could not save map file " + *i);
            }
         }
      }

      return success;
   }


   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::MapExists(const std::string& path)
   {
      return (osgDB::findDataFile(path) != "");
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::LoadMap(const std::string& path)
   {
      if(IsMapLoaded(path))
      {
         LOG_ERROR("Map already loaded: " + path);
         return false;
      }

      if(!MapExists(path))
      {
         LOG_ERROR("Map not found: " + path);
         return false;
      }

      MapBeginLoadMessage msg;
      msg.SetMapPath(path);
      GetEntityManager().EmitMessage(msg);

      bool success = mMapEncoder->LoadMapFromFile(path);
      if(success)
      {
         mLoadedMaps.insert(path);

         MapLoadedMessage msg1;
         msg1.SetMapPath(path);
         GetEntityManager().EmitMessage(msg1);
      }
      return success;
   }

   MapSystem::SpawnerStorage GetChildren(MapSystem::SpawnerStorage& spawners, const std::string& spawnername)
   {
      MapSystem::SpawnerStorage ret;
      MapSystem::SpawnerStorage::iterator i;
      for(i = spawners.begin(); i != spawners.end(); ++i)
      {
         Spawner* parent = i->second->GetParent();
         if((spawnername == "" && parent == NULL) ||
            (parent != NULL && parent->GetName() == spawnername))
         {
            ret[i->first] = i->second;
         }
      }
      return ret;
   }

   ////////////////////////////////////////////////////////////////////////////
   void MapSystem::EmitSpawnerDeleteMessages(MapSystem::SpawnerStorage& spawners, const std::string& path)
   {
      MapSystem::SpawnerStorage::iterator i;
      for(i = spawners.begin(); i != spawners.end(); ++i)
      {
         Spawner* spawner = i->second;

         if(spawner->GetMapName() == path)
         {
            MapSystem::SpawnerStorage children = GetChildren(mSpawners, spawner->GetName());
            EmitSpawnerDeleteMessages(children, path);
            SpawnerRemovedMessage msg;
            msg.SetName(i->first);
            msg.SetMapName(spawner->GetMapName());
            msg.SetCategory(spawner->GetGUICategory());
            GetEntityManager().EmitMessage(msg);
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::UnloadMap(const std::string& path)
   {
      if(!IsMapLoaded(path))
      {
         LOG_ERROR("Cannot unload map: not loaded! " + path);
         return false;
      }
      MapBeginUnloadMessage msg;
      msg.SetMapPath(path);
      GetEntityManager().EmitMessage(msg);

      // first collect entity ids of all entities in this map
      std::vector<EntityId> ids;
      GetEntitiesInMap(path, ids);

      std::vector<EntityId>::const_iterator j;
      for(j = ids.begin(); j != ids.end(); ++j)
      {
         GetEntityManager().RemoveFromScene(*j);
      }

      for(j = ids.begin(); j != ids.end(); ++j)
      {
         GetEntityManager().KillEntity(*j);
      }


      MapSystem::SpawnerStorage children = GetChildren(mSpawners, "");
      EmitSpawnerDeleteMessages(children, path);

      SpawnerStorage::iterator k = mSpawners.begin();
      while(k != mSpawners.end())
      {
         Spawner* spawner = k->second;
         if(spawner->GetMapName() == path)
         {
            mSpawners.erase(k++);
         }
         else
         {
            ++k;
         }
      }


      mLoadedMaps.erase(path);
      MapUnloadedMessage msg1;
      msg1.SetMapPath(path);
      GetEntityManager().EmitMessage(msg1);
      return true;
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::DeleteEntitiesByMap(const std::string& mapName)
   {
      if(!IsMapLoaded(mapName))
      {
         LOG_ERROR("Cannot unload map: not loaded! " + mapName);
         return false;
      }
     
      // first collect entity ids of all entities in this map
      std::vector<EntityId> ids;
      GetEntitiesInMap(mapName, ids);

      std::vector<EntityId>::const_iterator j;
      for(j = ids.begin(); j != ids.end(); ++j)
      {
         GetEntityManager().RemoveFromScene(*j);
      }

      for(j = ids.begin(); j != ids.end(); ++j)
      {
         GetEntityManager().KillEntity(*j);
      }
      return true;
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::SaveMap(const std::string& path)
   {
      if(!IsMapLoaded(path))
      {
         LOG_ERROR("Cannot save map: No map of this name exists!");
         return false;
      }
      std::string abspath = osgDB::findDataFile(path);
      if(abspath == "")
      {
         abspath = osgDB::getDataFilePathList().front() + "/" + path;
      }
      bool success = mMapEncoder->SaveMapToFile(path, abspath);
      
      return success;
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::SaveMapAs(const std::string& path, const std::string& copypath)
   {
      if(!IsMapLoaded(path))
      {
         LOG_ERROR("Cannot save map as: No map of this name exists!");
         return false;
      }
      bool success = mMapEncoder->SaveMapToFile(path, copypath);
      
      return success;
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::AddEmptyMap(const std::string& mapname)
   {
      if(IsMapLoaded(mapname))
      {
         return false;
      }
      if(osgDB::findDataFile(mapname) != "")
      {
         mLoadedMaps.insert(mapname);
         return false;
      }

      MapBeginLoadMessage msg;
      msg.SetMapPath(mapname);
      GetEntityManager().EmitMessage(msg);
      mLoadedMaps.insert(mapname);
      MapLoadedMessage msg2;
      msg2.SetMapPath(mapname);
      GetEntityManager().EmitMessage(msg2);
      return true;
   }

   ////////////////////////////////////////////////////////////////////////////
   bool MapSystem::IsMapLoaded(const std::string& path) const
   {
      return (mLoadedMaps.find(path) != mLoadedMaps.end());
   }

   ////////////////////////////////////////////////////////////////////////////
   std::vector<std::string> MapSystem::GetLoadedMaps() const
   {
      std::vector<std::string> ret;
      std::set<std::string>::const_iterator i;
      for(i = mLoadedMaps.begin(); i != mLoadedMaps.end(); ++i)
      {
         ret.push_back(*i);
      }
      return ret;
   }

   ////////////////////////////////////////////////////////////////////////////
   void MapSystem::GetEntitiesInMap(const std::string& mapname, std::vector<EntityId>& toFill) const
   {
      ComponentStore::const_iterator i;
      for(i = mComponents.begin(); i != mComponents.end(); ++i)
      {
         MapComponent* component = i->second;
         if(component->GetMapName() == mapname)
         {
            toFill.push_back(i->first);
         }         
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MapSystem::AddSpawner(Spawner& spawner)
   {
      SpawnerAddedMessage msg;

      mSpawners[spawner.GetName()] = &spawner;

      msg.SetName(spawner.GetName());
      if(spawner.GetParent() != NULL)
      {
         msg.SetParentName(spawner.GetParent()->GetName());
      }
      msg.SetMapName(spawner.GetMapName());

      GetEntityManager().EmitMessage(msg);
   }

   ///////////////////////////////////////////////////////////////////////////////
   bool MapSystem::DeleteSpawner(const std::string& name)
   {
      if(mSpawners.find(name) != mSpawners.end()) {
         SpawnerRemovedMessage msg;

         SpawnerStorage::iterator i = mSpawners.find(name);
         if(i == mSpawners.end()) return false;

         for(ComponentStore::iterator i = mComponents.begin(); i != mComponents.end(); ++i)
         {
            MapComponent* mc = i->second;
            if(mc->GetSpawnerName() == name)
            {
               mc->SetSpawnerName("");
            }
         }
         msg.SetName(name);

         msg.SetMapName(i->second->GetMapName());
         msg.SetCategory(i->second->GetGUICategory());
         mSpawners.erase(i);

         GetEntityManager().EmitMessage(msg);
         return true;
      }
      else
      {
         return false;
      }

   }

   ///////////////////////////////////////////////////////////////////////////////
   bool MapSystem::GetSpawner(const std::string& name, Spawner*& spawner) const
   {
      SpawnerStorage::const_iterator i = mSpawners.find(name);
      if(i == mSpawners.end()) {
         spawner = NULL;
         return false;
      }
      spawner = i->second.get();
      return true;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MapSystem::GetAllSpawners(std::map<std::string, Spawner*>& toFill) const
   {
      SpawnerStorage::const_iterator i = mSpawners.begin();
      for(; i != mSpawners.end(); ++i)
      {
         toFill[i->first] = i->second.get();
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MapSystem::GetAllSpawnerNames(std::vector<std::string>& toFill) const
   {
      SpawnerStorage::const_iterator i = mSpawners.begin();
      for(; i != mSpawners.end(); ++i)
      {
         toFill.push_back(i->first);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   bool MapSystem::Spawn(const std::string& name, Entity& spawned) const
   {
      Spawner* spawner;
      if(!GetSpawner(name, spawner))
      {
         return false;
      }
      return spawner->Spawn(spawned);
   }

   ///////////////////////////////////////////////////////////////////////////////
   EntityId MapSystem::GetEntityIdByUniqueId(const std::string& uniqueId) const
   {
      std::map<std::string, EntityId>::const_iterator i = mEntitiesByUniqueId.find(uniqueId);
      if(i == mEntitiesByUniqueId.end())
         return 0;
      else 
         return i->second;         
   }

   ///////////////////////////////////////////////////////////////////////////////
   bool MapSystem::GetEntityByUniqueId(const std::string& name, Entity*& entity) const
   {
      EntityId id = GetEntityIdByUniqueId(name);
      if(id > 0)
      {
         GetEntityManager().GetEntity(id, entity);
         return true;
      }
      return false;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MapSystem::OnSpawnEntity(const Message& msg)
   {
      const SpawnEntityMessage& m = static_cast<const SpawnEntityMessage&>(msg);
      Entity* entity;
      GetEntityManager().CreateEntity(entity);

      std::string spawnerName = m.GetSpawnerName();

      bool success = Spawn(spawnerName, *entity);
      if(!success)
      {
         LOG_ERROR("Could not spawn entity: spawner not found: " + spawnerName);
         return;
      }

      MapComponent* comp;
      if(!entity->GetComponent(comp))
      {
         entity->CreateComponent(comp);
      }
      comp->SetUniqueId(m.GetUniqueId());
      comp->SetEntityName(m.GetEntityName());
      if(m.GetAddToScene())
      {
         GetEntityManager().AddToScene(entity->GetId());
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MapSystem::OnDeleteEntity(const Message& msg)
   {
      const DeleteEntityMessage& m = static_cast<const DeleteEntityMessage&>(msg);

      Entity* entity;
      bool success = GetEntityByUniqueId(m.GetUniqueId(), entity);
      if(!success)
      {
         LOG_ERROR("Cannot delete: Entity with unique id not found: " + m.GetUniqueId());
         return;
      }
      GetEntityManager().RemoveFromScene(entity->GetId());
      GetEntityManager().KillEntity(entity->GetId());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MapSystem::GetSpawnerCreatedEntities(const std::string& spawnername, std::vector<EntityId>& ids) const
   {
      ComponentStore::const_iterator i;
      for(i = mComponents.begin(); i != mComponents.end(); ++i)
      {
         MapComponent* component = i->second;
         if(component->GetSpawnerName() == spawnername)
         {
            ids.push_back(i->first);
         }
      }
   }
}