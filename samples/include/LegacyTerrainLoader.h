/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreConfigFile.h>

namespace Ogre
{
class ManualTerrainMaterial : public Ogre::TerrainMaterialGenerator
{
    MaterialPtr mMaterial;
    bool mIsInit;
    bool mNormalMapRequired;
public:
    ManualTerrainMaterial(const String& matName) : mIsInit(false), mNormalMapRequired(true)
    {
        auto terrainGlobals = TerrainGlobalOptions::getSingletonPtr();
        mMaterial =
            MaterialManager::getSingleton().getByName(matName, terrainGlobals->getDefaultResourceGroup());
    }

    void setNormalMapRequired(bool enable) { mNormalMapRequired = enable; }

    MaterialPtr generate(const Terrain* terrain) override
    {
        if (!mIsInit && mNormalMapRequired)
        {
            // Get default pass
            Pass *p = mMaterial->getTechnique(0)->getPass(0);	

            // Add terrain's global normalmap to renderpass so the fragment program can find it.
            p->createTextureUnitState()->_setTexturePtr(terrain->getTerrainNormalMap());

        }
        mIsInit = true;

        return mMaterial;
    }

    bool isVertexCompressionSupported() const override { return false; }

    void requestOptions(Terrain* terrain) override
    {
        terrain->_setLightMapRequired(false);
        terrain->_setCompositeMapRequired(false);
        terrain->_setNormalMapRequired(mNormalMapRequired); 
    }
};
} // namespace Ogre

inline Ogre::TerrainGroup* loadLegacyTerrain(const Ogre::String& cfgFileName, Ogre::SceneManager* sceneMgr)
{
    using namespace Ogre;

    auto terrainGroup = new TerrainGroup(sceneMgr);

    ConfigFile cfg;
    cfg.loadFromResourceSystem(cfgFileName, terrainGroup->getResourceGroup());

    auto terrainGlobals = TerrainGlobalOptions::getSingletonPtr();
    if(!terrainGlobals)
        terrainGlobals = new TerrainGlobalOptions();

    const String& customMatName = cfg.getSetting("CustomMaterialName");

    if(!customMatName.empty())
    {
        auto matgen = std::make_shared<ManualTerrainMaterial>(customMatName);
        matgen->setNormalMapRequired(StringConverter::parseBool(cfg.getSetting("VertexNormals")));
        terrainGlobals->setDefaultMaterialGenerator(matgen);
    }

#if OGRE_VERSION >= ((1 << 16) | (11 << 8) | 6)
    terrainGroup->loadLegacyTerrain(cfg);
#endif

    return terrainGroup;
}