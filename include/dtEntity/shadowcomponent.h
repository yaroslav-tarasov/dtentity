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

#include <osg/ref_ptr>
#include <dtEntity/export.h>
#include <dtEntity/defaultentitysystem.h>
#include <dtEntity/osgcomponents.h>
#include <dtEntity/stringid.h>
#include <osg/Geode>
#include <osgShadow/ShadowMap>

namespace dtEntity
{

   /**
    * Holds a single OSG node.
    */
   class DT_ENTITY_EXPORT ShadowComponent : public GroupComponent
   {
      typedef GroupComponent BaseClass;
      
   public:
      
      static const ComponentType TYPE;
      static const StringId ShadowTechniqueId;
      static const StringId MinLightMarginId;
      static const StringId MaxFarPlaneId;
      static const StringId TexSizeId;
      static const StringId BaseTexUnitId;
      static const StringId ShadowTexUnitId;
      static const StringId ShadowTexCoordIndexId;
      static const StringId BaseTexCoordIndexId;
      static const StringId PSSMMapCountId;
      static const StringId PSSMMapResId;
      static const StringId PSSMMapDebugColorOnId;
      static const StringId PSSMMinNearSplitId;
      static const StringId PSSMMaxFarDistId;
      static const StringId PSSMMoveVCamFactorId;
      static const StringId PSSMPolyOffsetFactorId;
      static const StringId PSSMPolyOffsetUnitId;

      ShadowComponent();
      virtual ~ShadowComponent();

      void OnAddedToEntity(Entity& entity);

      virtual ComponentType GetType() const { return TYPE; }

      virtual void OnFinishedSettingProperties();

      void SetShadowTechnique(const std::string name);

   private:

      Entity* mEntity;
      osg::ref_ptr<osgShadow::ShadowTechnique> mTechnique;
      
      StringProperty mShadowTechnique;
      FloatProperty mMinLightMargin;
      FloatProperty mMaxFarPlane;
      UIntProperty mTexSize;
      UIntProperty mBaseTexUnit;
      UIntProperty mShadowTexUnit;
      UIntProperty mShadowTexCoordIndex;
      UIntProperty mBaseTexCoordIndex;

      UIntProperty mPSSMMapCount;
      UIntProperty mPSSMMapRes;
      BoolProperty mPSSMMapDebugColorOn;
      UIntProperty mPSSMMinNearSplit;
      UIntProperty mPSSMMaxFarDist;
      UIntProperty mPSSMMoveVCamFactor;
      FloatProperty mPSSMPolyOffsetFactor;
      FloatProperty mPSSMPolyOffsetUnit;
   };

  
   //////////////////////////////////////////////////////////

   // storage only
   class DT_ENTITY_EXPORT ShadowSystem
      : public DefaultEntitySystem<ShadowComponent>
   {
   public:
      ShadowSystem(EntityManager& em);
   };

}
