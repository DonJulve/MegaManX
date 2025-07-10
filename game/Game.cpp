#include "Game.h"
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "Engine.h"
#include "Systems.h"
#include "Componentes_prueba/ComponenteDePrueba.h"
#include "Componentes_prueba/ComponenteDePrueba2.h"
#include "megaman/MegamanController.h"
#include "megaman/MegamanBulletComponent.h"
#include "megaman/MegamanMediumBulletComponent.h"
#include "megaman/MegamanStrongBulletComponent.h"
#include "megaman/MegamanCamera.h"
#include "megamanArmor/MegamanArmorController.h"
#include "chillPenguin/ChillPenguinIA.h"
#include "chillPenguin/ChillPenguinBlizzardComponent.h"
#include "chillPenguin/ChillPenguinBulletComponent.h"
#include "chillPenguin/ChillPenguinIceBulletComponent.h"
#include "chillPenguin/ChillPenguinPenguinComponent.h"
#include "enemies/RayBit/RaybitController.h"
#include "effects/SmallExplosionComponent.h"
#include "effects/BigExplosionComponent.h"
#include "enemies/axeMax/AxeMaxController.h"
#include "enemies/axeMax/BaseComponent.h"
#include "enemies/axeMax/BlockComponent.h"
#include "enemies/RayBit/RaybitBulletComponent.h"
#include "enemies/bombBeen/BombBeenController.h"
#include "enemies/bombBeen/BombBeenMineComponent.h"
#include "enemies/battonBone/BattonBoneController.h"
#include "enemies/tombot/TombotController.h"
#include "enemies/tombot/TombotBootsController.h"
#include "enemies/SnowShooter/SnowShooterController.h"
#include "enemies/SnowShooter/SnowShooterBulletComponent.h"
#include "enemies/Flammingle/FlammingleController.h"
#include "enemies/Flammingle/FlammingleBulletComponent.h"
#include "enemies/jamminger/JammingerController.h"
#include "enemies/spiky/SpikyController.h"
#include "enemies/scrapRobo/ScrapRoboController.h"
#include "enemies/scrapRobo/ScrapRoboBullet.h"
#include "enemies/digLabour/DigLabourPickAxe.h"
#include "enemies/digLabour/DigLabourController.h"
#include "enemies/acidDrop/AcidDropGenerator.h"
#include "enemies/acidDrop/AcidDropComponent.h"
#include "enemies/pipeturn/PipeturnController.h"
#include "enemies/Hoganmer/HoganmerController.h"
#include "enemies/Hoganmer/HoganmerBall.h"
#include "menus/uiComponents/FlammeMenuComponent.h"
#include "CameraDebugController.h"
#include "Components.h"
#include "SceneLoader.h"
#include "HealthComponent.h"
#include "MegamanLevelsManager.h"
#include "menus/MainMenuComponent.h"
#include "menus/BossSelectMenuComponent.h"
#include "menus/uiComponents/MegamanMainMenuComponent.h"
#include "menus/uiComponents/BulletMainMenuComponent.h"
#include "menus/uiComponents/SelectorMenuComponent.h"
#include "menus/uiComponents/BossMenuComponent.h"
#include "menus/uiComponents/PointMapMenuComponent.h"
#include "menus/uiComponents/MapMenuComponent.h"
#include "menus/uiComponents/StageMenuComponent.h"
#include "menus/uiComponents/SpecMenuComponent.h"
#include "menus/uiComponents/MapBgMenuComponent.h"
#include "menus/uiComponents/FullBarMenuComponent.h"
#include "menus/uiComponents/MiniBarMenuComponent.h"
#include "menus/PauseMenuComponent.h"
#include "menus/uiComponents/PowerMenuComponent.h"
#include "menus/OptionsMenuComponent.h"
#include "drops/DropController.h"
#include "levels/ChillPenguinManager.h"
#include "levels/FlameMammothManager.h"
#include "bars/BarLifeComponent.h"
#include "bars/BarPowerUpsComponent.h"
#include "structures/IglooController.h"
#include "Debug.h"
#include "flamemammoth/Fire.h"
#include "flamemammoth/Lava.h"
#include "flamemammoth/FlamemammothController.h"
#include "structures/Crusher.h"
#include "megaman/MegamanCharge.h"
#include "enemies/skyClaw/SkyClaw.h"
#include "menus/uiComponents/FlammeMenuComponent.h"
#include "enemies/metal/Metal.h"
#include "flamemammoth/FlamemammothAdvancedIA.h"
#include "chillPenguin/ChillPenguinAdvancedIA.h"

// Constructor: Currently empty as per original implementation
Game::Game(const char *title, int width, int height) {}

// Registers components with the SceneLoader
void Game::registro()
{
    SceneLoader *sl = &(SceneLoader::getInstance());

    sl->registerComponentLoader<DropController, DropControllerParameters>(
        "DropController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> DropController *
        {
            return new DropController(DropControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<ComponenteDePrueba, ComponenteDePruebaParameters>(
        "CDP",
        [](const nlohmann::json &j, EntityManager &entityManager) -> ComponenteDePrueba *
        {
            return new ComponenteDePrueba(ComponenteDePruebaLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<CameraDebugController, CameraDebugControllerParameters>(
        "CDC",
        [](const nlohmann::json &j, EntityManager &entityManager) -> CameraDebugController *
        {
            return new CameraDebugController(CameraDebugControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MegamanController, MegamanControllerParameters>(
        "MegamanController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MegamanController *
        {
            return new MegamanController(MegamanControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MegamanArmorController, MegamanArmorControllerParameters>(
        "MegamanArmorController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MegamanArmorController *
        {
            return new MegamanArmorController(MegamanArmorControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MegamanBulletComponent, MegamanBulletParameters>(
        "MegamanBulletComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MegamanBulletComponent *
        {
            return new MegamanBulletComponent(MegamanBulletComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MegamanMediumBulletComponent, MegamanMediumBulletParameters>(
        "MegamanMediumBulletComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MegamanMediumBulletComponent *
        {
            return new MegamanMediumBulletComponent(MegamanMediumBulletComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MegamanStrongBulletComponent, MegamanStrongBulletParameters>(
        "MegamanStrongBulletComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MegamanStrongBulletComponent *
        {
            return new MegamanStrongBulletComponent(MegamanStrongBulletComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<ChillPenguinController, ChillPenguinControllerParameters>(
        "ChillPenguinController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> ChillPenguinController *
        {
            return new ChillPenguinController(ChillPenguinControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<ChillPenguinBlizzardComponent, ChillPenguinBlizzardParameters>(
        "ChillPenguinBlizzardComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> ChillPenguinBlizzardComponent *
        {
            return new ChillPenguinBlizzardComponent(ChillPenguinBlizzardComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<ChillPenguinBulletComponent, ChillPenguinBulletParameters>(
        "ChillPenguinBulletComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> ChillPenguinBulletComponent *
        {
            return new ChillPenguinBulletComponent(ChillPenguinBulletComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<ChillPenguinIceBulletComponent, ChillPenguinIceBulletParameters>(
        "ChillPenguinIceBulletComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> ChillPenguinIceBulletComponent *
        {
            return new ChillPenguinIceBulletComponent(ChillPenguinIceBulletComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<ChillPenguinPenguinComponent, ChillPenguinPenguinParameters>(
        "ChillPenguinPenguinComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> ChillPenguinPenguinComponent *
        {
            return new ChillPenguinPenguinComponent(ChillPenguinPenguinComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<RaybitController, RaybitControllerParameters>(
        "RaybitController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> RaybitController *
        {
            return new RaybitController(RaybitControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<RaybitBulletComponent, RaybitBulletParameters>(
        "RaybitBulletComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> RaybitBulletComponent *
        {
            return new RaybitBulletComponent(RaybitBulletComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<SmallExplosionComponent, SmallExplosionParameters>(
        "SmallExplosionComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> SmallExplosionComponent *
        {
            return new SmallExplosionComponent(SmallExplosionLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<BigExplosionComponent, BigExplosionParameters>(
        "BigExplosionComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> BigExplosionComponent *
        {
            return new BigExplosionComponent(BigExplosionLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<BombBeenController, BombBeenControllerParameters>(
        "BombBeenController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> BombBeenController *
        {
            return new BombBeenController(BombBeenControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<BombBeenMineComponent, BombBeenMineParameters>(
        "BombBeenMineComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> BombBeenMineComponent *
        {
            return new BombBeenMineComponent(BombBeenMineLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<AxeMaxController, AxeMaxControllerParameters>(
        "AxeMaxController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> AxeMaxController *
        {
            return new AxeMaxController(AxeMaxControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<BaseComponent, BaseComponentParameters>(
        "BaseComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> BaseComponent *
        {
            return new BaseComponent(BaseComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<BlockComponent, BlockComponentParameters>(
        "BlockComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> BlockComponent *
        {
            return new BlockComponent(BlockComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<BattonBoneController, BattonBoneControllerParameters>(
        "BattonBoneController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> BattonBoneController *
        {
            return new BattonBoneController(BattonBoneControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<SnowShooterController, SnowShooterControllerParameters>(
        "SnowShooterController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> SnowShooterController *
        {
            return new SnowShooterController(SnowShooterControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<SnowShooterBulletComponent, SnowShooterBulletParameters>(
        "SnowShooterBulletComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> SnowShooterBulletComponent *
        {
            return new SnowShooterBulletComponent(SnowShooterBulletComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<FlammingleController, FlammingleControllerParameters>(
        "FlammingleController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> FlammingleController *
        {
            return new FlammingleController(FlammingleControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<FlammingleBulletComponent, FlammingleBulletParameters>(
        "FlammingleBulletComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> FlammingleBulletComponent *
        {
            return new FlammingleBulletComponent(FlammingleBulletComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<SpikyController, SpikyControllerParameters>(
        "SpikyController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> SpikyController *
        {
            return new SpikyController(SpikyControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MainMenuComponent, MainMenuComponentParameters>(
        "MainMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MainMenuComponent *
        {
            return new MainMenuComponent(MainMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<HealthComponent, HealthComponentParameters>(
        "Health",
        [](const nlohmann::json &j, EntityManager &entityManager) -> HealthComponent *
        {
            return new HealthComponent(HealthComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<BossSelectComponent, BossSelectComponentParameters>(
        "BossSelectMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> BossSelectComponent *
        {
            return new BossSelectComponent(BossSelectComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MegamanMainMenuComponent, MegamanMainMenuComponentParameters>(
        "MegamanMainMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MegamanMainMenuComponent *
        {
            return new MegamanMainMenuComponent(MegamanMainMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<BulletMainMenuComponent, BulletMainMenuComponentParameters>(
        "BulletMainMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> BulletMainMenuComponent *
        {
            return new BulletMainMenuComponent(BulletMainMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<SelectorMenuComponent, SelectorMenuComponentParameters>(
        "SelectorMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> SelectorMenuComponent *
        {
            return new SelectorMenuComponent(SelectorMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<BossMenuComponent, BossMenuComponentParameters>(
        "BossMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> BossMenuComponent *
        {
            return new BossMenuComponent(BossMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<PointMapMenuComponent, PointMapMenuComponentParameters>(
        "PointMapMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> PointMapMenuComponent *
        {
            return new PointMapMenuComponent(PointMapMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MapMenuComponent, MapMenuComponentParameters>(
        "MapMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MapMenuComponent *
        {
            return new MapMenuComponent(MapMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<StageMenuComponent, StageMenuComponentParameters>(
        "StageMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> StageMenuComponent *
        {
            return new StageMenuComponent(StageMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<SpecMenuComponent, SpecMenuComponentParameters>(
        "SpecMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> SpecMenuComponent *
        {
            return new SpecMenuComponent(SpecMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MapBgMenuComponent, MapBgMenuComponentParameters>(
        "MapBgMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MapBgMenuComponent *
        {
            return new MapBgMenuComponent(MapBgMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<JammingerController, JammingerControllerParameters>(
        "JammingerController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> JammingerController *
        {
            return new JammingerController(JammingerControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MegamanLevelsManager, MegamanLevelsManagerParameters>(
        "MegamanLevelsManager",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MegamanLevelsManager *
        {
            return new MegamanLevelsManager(MegamanLevelsManagerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<PauseMenuComponent, PauseMenuComponentParameters>(
        "PauseMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> PauseMenuComponent *
        {
            return new PauseMenuComponent(PauseMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<FullBarMenuComponent, FullBarMenuComponentParameters>(
        "BarMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> FullBarMenuComponent *
        {
            return new FullBarMenuComponent(FullBarMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MegamanCamera, MegamanCameraParameters>(
        "MegamanCamera",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MegamanCamera *
        {
            return new MegamanCamera(MegamanCameraLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<PowerMenuComponent, PowerMenuComponentParameters>(
        "PowerMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> PowerMenuComponent *
        {
            return new PowerMenuComponent(PowerMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MiniBarMenuComponent, MiniBarMenuComponentParameters>(
        "MiniBarMenu",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MiniBarMenuComponent *
        {
            return new MiniBarMenuComponent(MiniBarMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<OptionsMenuComponent, OptionsMenuComponentParameters>(
        "OptionsMenuComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> OptionsMenuComponent *
        {
            return new OptionsMenuComponent(OptionsMenuComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<ChillPenguinManager, ChillPenguinManagerParameters>(
        "ChillPenguinManager",
        [](const nlohmann::json &j, EntityManager &entityManager) -> ChillPenguinManager *
        {
            return new ChillPenguinManager(ChillPenguinManagerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<BarLifeComponent, BarLifeComponentParameters>(
        "BarLifeComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> BarLifeComponent *
        {
            return new BarLifeComponent(BarLifeComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<BarPowerUpsComponent, BarPowerUpsComponentParameters>(
        "BarPowerUpsComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> BarPowerUpsComponent *
        {
            return new BarPowerUpsComponent(BarPowerUpsComponentLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<TombotController, TombotControllerParameters>(
        "TombotController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> TombotController *
        {
            return new TombotController(TombotControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<TombotBootsController, TombotBootsControllerParameters>(
        "TombotBootsController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> TombotBootsController *
        {
            return new TombotBootsController(TombotBootsControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<IglooController, IglooControllerParameters>(
        "IglooController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> IglooController *
        {
            return new IglooController(IglooControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<ScrapRoboController, ScrapRoboControllerParameters>(
        "ScrapRoboController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> ScrapRoboController *
        {
            return new ScrapRoboController(ScrapRoboControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<ScrapRoboBullet, ScrapRoboBulletParameters>(
        "ScrapRoboBullet",
        [](const nlohmann::json &j, EntityManager &entityManager) -> ScrapRoboBullet *
        {
            return new ScrapRoboBullet(ScrapRoboBulletLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<DigLabourPickAxe, DigLabourPickAxeParameters>(
        "DigLabourPickAxe",
        [](const nlohmann::json &j, EntityManager &entityManager) -> DigLabourPickAxe *
        {
            return new DigLabourPickAxe(DigLabourPickAxeLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<DigLabourController, DigLabourControllerParameters>(
        "DigLabour",
        [](const nlohmann::json &j, EntityManager &entityManager) -> DigLabourController *
        {
            return new DigLabourController(DigLabourControllerLoader::fromJSON(j, entityManager));
        });

    // sl->registerComponentLoader<AcidDropGenerator, AcidDropGeneratorParameters>(
    //     "AcidDropGenerator",
    //     [](const nlohmann::json &j, EntityManager &entityManager) -> AcidDropGenerator *
    //     {
    //         return new AcidDropGenerator(AcidDropGeneratorLoader::fromJSON(j, entityManager));
    //     });

    // sl->registerComponentLoader<AcidDropComponent, AcidDropComponentParameters>(
    //     "AcidDrop",
    //     [](const nlohmann::json &j, EntityManager &entityManager) -> AcidDropComponent *
    //     {
    //         return new AcidDropComponent(AcidDropComponentLoader::fromJSON(j, entityManager));
    //     });

    sl->registerComponentLoader<PipeturnController, PipeturnControllerParameters>(
        "PipeturnController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> PipeturnController *
        {
            return new PipeturnController(PipeturnControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<HoganmerController, HoganmerControllerParameters>(
        "HoganmerController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> HoganmerController *
        {
            return new HoganmerController(HoganmerControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<HoganmerBall, HoganmerBallParameters>(
        "HoganmerBall",
        [](const nlohmann::json &j, EntityManager &entityManager) -> HoganmerBall *
        {
            return new HoganmerBall(HoganmerBallLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<LavaComponent, LavaParameters>(
        "FlameMammothLava",
        [](const nlohmann::json &j, EntityManager &entityManager) -> LavaComponent *
        {
            return new LavaComponent(LavaLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<FireComponent, FireParameters>(
        "FlameMammothFire",
        [](const nlohmann::json &j, EntityManager &entityManager) -> FireComponent *
        {
            return new FireComponent(FireLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<FlamemammothController, FlamemammothControllerParameters>(
        "FlameMammothController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> FlamemammothController *
        {
            return new FlamemammothController(FlamemammothControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<FlameMammothManager, FlameMammothManagerParameters>(
        "FlameMammothManager",
        [](const nlohmann::json &j, EntityManager &entityManager) -> FlameMammothManager *
        {
            return new FlameMammothManager(FlameMammothManagerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<Crusher, CrusherParameters>(
        "Crusher",
        [](const nlohmann::json &j, EntityManager &entityManager) -> Crusher *
        {
            return new Crusher(CrusherLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<MegamanChargeComponent, MegamanChargeParameters>(
        "MegamanCharge",
        [](const nlohmann::json &j, EntityManager &entityManager) -> MegamanChargeComponent *
        {
            return new MegamanChargeComponent(MegamanChargeLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<SkyClawController, SkyClawControllerParameters>(
        "SkyclawController",
        [](const nlohmann::json &j, EntityManager &entityManager) -> SkyClawController *
        {
            return new SkyClawController(SkyClawControllerLoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<FlammeMenuComponent, FlammeMenuComponentParameters>(
        "FlammeMenuComponent",
        [](const nlohmann::json &j, EntityManager &entityManager) -> FlammeMenuComponent *
        {
            return new FlammeMenuComponent(FlammeMenuComponentLoader::fromJSON(j, entityManager));
        });

    // sl->registerComponentLoader<MetalController, MetalControllerParameters>(
    //     "MetalController",
    //     [](const nlohmann::json &j, EntityManager &entityManager) -> MetalController *
    //     {
    //         return new MetalController(MetalControllerLoader::fromJSON(j, entityManager));
    //     });

    sl->registerComponentLoader<FlamemammothAdvancedIA, FlamemammothAdvancedIAParameters>(
        "FlameMammothControllerAdvancedIA",
        [](const nlohmann::json &j, EntityManager &entityManager) -> FlamemammothAdvancedIA *
        {
            return new FlamemammothAdvancedIA(FlamemammothAdvancedIALoader::fromJSON(j, entityManager));
        });

    sl->registerComponentLoader<ChillPenguinAdvancedIA, ChillPenguinAdvancedIAParameters>(
        "ChillPenguinAdvancedIA",
        [](const nlohmann::json &j, EntityManager &entityManager) -> ChillPenguinAdvancedIA *
        {
            return new ChillPenguinAdvancedIA(ChillPenguinAdvancedIALoader::fromJSON(j, entityManager));
        });
}

// Runs the game loop
void Game::run()
{
    try
    {
        // Initialize the engine
        Engine *engine = Engine::getInstance("Megaman", 800, 600);

        registro();

        // Enable logging to a file
        Debug::EnableFileLogging("debug.log");
        // Load scene
        engine->getSceneManager()->addScene("loading_scene");
        engine->getSceneManager()->addScene("mainMenu");
        engine->getSceneManager()->addScene("pauseMenu");


        // Run the game loop
        engine->run();

        // Disable logging when done
        Debug::DisableFileLogging();
    }
    catch (const std::exception &e)
    {
        Debug::Error(std::string("Unhandled Exception: ") + e.what());
    }
    catch (...)
    {
        Debug::Error("Unknown exception caught!");
    }
}