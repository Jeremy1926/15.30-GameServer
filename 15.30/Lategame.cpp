#include "pch.h"
#include "Lategame.h"
#include "Utils.h"

FItemAndCount Lategame::GetShotguns()
{
    static UEAllocatedVector<FItemAndCount> Shotguns
    {
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Swing_Athena_VR.WID_Shotgun_Swing_Athena_VR")), // epic lever
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Swing_Athena_SR.WID_Shotgun_Swing_Athena_SR")), // legendary lever
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03")),
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_VR_Ore_T03.WID_Shotgun_Standard_Athena_VR_Ore_T03")),
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_SR_Ore_T03.WID_Shotgun_Standard_Athena_SR_Ore_T03")),
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Combat_Athena_SR_Ore_T03.WID_Shotgun_Combat_Athena_SR_Ore_T03")),
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Swing_Athena_SR.WID_Shotgun_Swing_Athena_SR")),
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Charge_Athena_UR_Ore_T03.WID_Shotgun_Charge_Athena_UR_Ore_T03")),
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_SemiAuto_Athena_R_Ore_T03.WID_Shotgun_SemiAuto_Athena_R_Ore_T03")),
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_SlugFire_Athena_SR.WID_Shotgun_SlugFire_Athena_SR")),
    };

    return Shotguns[rand() % (Shotguns.size() - 1)];
}

FItemAndCount Lategame::GetAssaultRifles()
{
    static UEAllocatedVector<FItemAndCount> AssaultRifles
    {
         FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03")),
         FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_VR_Ore_T03.WID_Assault_AutoHigh_Athena_VR_Ore_T03")),
         FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_R_Ore_T03.WID_Assault_Heavy_Athena_R_Ore_T03")),
         FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_UC_Ore_T03.WID_Assault_Heavy_Athena_UC_Ore_T03")),
         FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_Infantry_Athena_R.WID_Assault_Infantry_Athena_R")),
         FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_PistolCaliber_AR_Athena_SR_Ore_T03.WID_Assault_PistolCaliber_AR_Athena_SR_Ore_T03")),
         FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_SemiAuto_Athena_UR_Ore_T03.WID_Assault_SemiAuto_Athena_UR_Ore_T03")),
         FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_SemiAuto_Athena_VR_Ore_T03.WID_Assault_SemiAuto_Athena_VR_Ore_T03")),
         FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_Suppressed_Athena_SR_Ore_T03.WID_Assault_Suppressed_Athena_SR_Ore_T03")),
         FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_Surgical_Athena_R_Ore_T03.WID_Assault_Surgical_Athena_R_Ore_T03")),
         FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/Boss/WID_Boss_Adventure_AR.WID_Boss_Adventure_AR")),
    };
    return AssaultRifles[rand() % (AssaultRifles.size() - 1)];
}


FItemAndCount Lategame::GetSnipers()
{
    static UEAllocatedVector<FItemAndCount> Heals
    {
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_VR_Ore_T03.WID_Sniper_Heavy_Athena_VR_Ore_T03")), // heavy sniper
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/CosmosGameplay/Items/CosmosRifle/WID_Sniper_Cosmos_Athena_VR.WID_Sniper_Cosmos_Athena_VR")), // mandalorion sniper
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WaffleTruck/WID_WaffleTruck_BoomSniper.WID_WaffleTruck_BoomSniper")), // boom sniper
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Sniper_NoScope_Athena_R_Ore_T03.WID_Sniper_NoScope_Athena_R_Ore_T03")), // boom sniper
    };

    return Heals[rand() % (Heals.size() - 1)];
}

FItemAndCount Lategame::GetHeals()
{
    static UEAllocatedVector<FItemAndCount> Heals
    {
        FItemAndCount(3, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/Shields/Athena_Shields.Athena_Shields")), // big pots
        FItemAndCount(3, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/Medkit/Athena_Medkit.Athena_Medkit")), // medkit
        FItemAndCount(6, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall")), // minis
        //FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/LTM/WID_Hook_Gun_Slide.WID_Hook_Gun_Slide")), // grappler
    };

    return Heals[rand() % (Heals.size() - 1)];
}

FItemAndCount Lategame::GetExtra()
{
    static UEAllocatedVector<FItemAndCount> Heals
    {
        FItemAndCount(6, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/ShockwaveGrenade/Athena_ShockGrenade.Athena_ShockGrenade")), // shockwave
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/SuperTowerGrenade/Levels/PortAFort_A/Athena_SuperTowerGrenade_A.Athena_SuperTowerGrenade_A")), // Port-A-Fort
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/Boss/WID_Boss_GhostMidas.WID_Boss_GhostMidas")), // Midas DrumGun
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03")),
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WaffleTruck/WID_WaffleTruck_HopRockDualies.WID_WaffleTruck_HopRockDualies")),
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/Boss/WID_Boss_Midas.WID_Boss_Midas")),
        FItemAndCount(3, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/AppleSun/WID_Athena_AppleSun.WID_Athena_AppleSun")),
        FItemAndCount(5, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/Balloons/Athena_Balloons_Consumable.Athena_Balloons_Consumable")),
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavy_Athena_UC_Ore_T03.WID_Pistol_AutoHeavy_Athena_UC_Ore_T03")),
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Pistol_HandCannon_Tech_Athena_VR_Ore_T03.WID_Pistol_HandCannon_Tech_Athena_VR_Ore_T03")),
    };

    return Heals[rand() % (Heals.size() - 1)];
}

UFortAmmoItemDefinition* Lategame::GetAmmo(EAmmoType AmmoType)
{
    static UEAllocatedVector<UFortAmmoItemDefinition*> Ammos
    {
        Utils::FindObject<UFortAmmoItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight"),
        Utils::FindObject<UFortAmmoItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells"),
        Utils::FindObject<UFortAmmoItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium"),
        Utils::FindObject<UFortAmmoItemDefinition>(L"/Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets"),
        Utils::FindObject<UFortAmmoItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy")
    };

    return Ammos[(uint8)AmmoType];
}

UFortResourceItemDefinition* Lategame::GetResource(EFortResourceType ResourceType)
{
    static UEAllocatedVector<UFortResourceItemDefinition*> Resources
    {
        Utils::FindObject<UFortResourceItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData"),
        Utils::FindObject<UFortResourceItemDefinition>(L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData"),
        Utils::FindObject<UFortResourceItemDefinition>(L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData")
    };

    return Resources[(uint8)ResourceType];
}