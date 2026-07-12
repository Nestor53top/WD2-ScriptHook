#include "game_api.h"
#include "pattern_scan.h"
#include "hook_manager.h"

typedef struct _GAME_FUNC_PTRS {
    SpawnEntity_t                   SpawnEntityFromArchetypeHash;
    SetProgression_t                SetProgression;
    ShowNotification_t              ShowNotification;
    CameraSetCustom_t               CameraSetCustom;
    GetPlayerPosition_t             GetPlayerPosition;
    GetLocalPlayerEntityId_t        GetLocalPlayerEntityId;
    SetLocalPlayerNoclip_t          SetLocalPlayerNoclip;
    HasLocalPlayerNoclip_t          HasLocalPlayerNoclip;
    SetLocalPlayerFreeCamera_t      SetLocalPlayerFreeCamera;
    HasLocalPlayerFreeCamera_t      HasLocalPlayerFreeCamera;
    Teleport_t                      Teleport;
    RepairVehicle_t                 RepairVehicle;
    EnableVehicleEngine_t           EnableVehicleEngine;
    IsVehicleEngineEnabled_t        IsVehicleEngineEnabled;
    EnableVehicleSiren_t            EnableVehicleSiren;
    IsVehicleSirenEnabled_t         IsVehicleSirenEnabled;
    EnableVehicleEmergencyLight_t   EnableVehicleEmergencyLight;
    EnableVehicleIndicator_t        EnableVehicleIndicator;
    EnableVehicleSpotLight_t        EnableVehicleSpotLight;
    EnableVehicleHighBeamLights_t   EnableVehicleHighBeamLights;
    EnableVehicleConvenienceLight_t EnableVehicleConvenienceLight;
    SetVehicleLicensePlateText_t    SetVehicleLicensePlateText;
    SetVehicleLightBehaviorFlag_t   SetVehicleLightBehaviorFlag;
    GetVehiclePartCount_t           GetVehiclePartCount;
    GetVehiclePartHash_t            GetVehiclePartHash;
    GetVehiclePartPosition_t        GetVehiclePartPosition;
    GetVehiclePartRotation_t        GetVehiclePartRotation;
    SetVehiclePartPosition_t        SetVehiclePartPosition;
    SetVehiclePartRotation_t        SetVehiclePartRotation;
    PutPlayerInVehicleDelayed_t     PutPlayerInVehicleDelayed;
    FelonyStartChase_t              FelonyStartChase;
    ToggleFelonySystem_t            ToggleFelonySystem;
    ClearHeatLevel_t                ClearHeatLevel;
    SetHeatLevel_t                  SetHeatLevel;
    SetMinHeatLevel_t               SetMinHeatLevel;
    SetSearchHeat_t                 SetSearchHeat;
    GetGlobalTrafficLightState_t    GetGlobalTrafficLightState;
    SetGlobalTrafficLightState_t    SetGlobalTrafficLightState;
    ToggleWorldImpostor_t           ToggleWorldImpostor;
    ToggleWorldSpawner_t            ToggleWorldSpawner;
    IsWorldImpostorEnabled_t        IsWorldImpostorEnabled;
    IsWorldSpawnerEnabled_t         IsWorldSpawnerEnabled;
    SetFrameRate_t                  SetFrameRate;
    GetFrameRate_t                  GetFrameRate;
    DetectGameVersion_t             DetectGameVersion;
    RefreshKeybinds_t               RefreshKeybinds;
    RefreshKeybind_t                RefreshKeybind;
    PlayBroadcast_t                 PlayBroadcast;
    ResetBroadcastToDefault_t       ResetBroadcastToDefault;
    KillSplashscreen_t              KillSplashscreen;
    IsIngameUIEnabled_t             IsIngameUIEnabled;
    SetLocalPlayerGraphicKitModel_t SetLocalPlayerGraphicKitModel;
    SetEntityGraphicKitModel_t      SetEntityGraphicKitModel;
    GetGraphicKitModelByEntityId_t  GetGraphicKitModelByEntityId;
    ForceHackIngredient_t           ForceHackIngredient;
    ExecuteReward_V2_t              ExecuteReward_V2;
    CDominoManager_GetInstance_t    CDominoManager_GetInstance;
    PlaySound_t                     PlaySound;
    GetAllEntities_t                GetAllEntities;
    GetEntitiesInRange_t            GetEntitiesInRange;
    GetEntitiesWithComponent_t      GetEntitiesWithComponent;
    EntityHasComponent_t            EntityHasComponent;
    GetEntityClassName_t            GetEntityClassName;
    GetEntityComponents_t           GetEntityComponents;
} GAME_FUNC_PTRS;

static GAME_FUNC_PTRS g_gameFuncs = {0};

static const char *GetEntityClassName_Signature = "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x57\x48\x83\xEC\x20\x8B\xDA";
static const char *GetEntityClassName_Mask = "xxxx??xxxxxxx";

static const char *SpawnEntity_Signature = "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x56\x57\x41\x56\x41\x57";
static const char *SpawnEntity_Mask = "xxxx??xxxxxxx";

static const char *RepairVehicle_Signature = "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x8B\xDA";
static const char *RepairVehicle_Mask = "xxxx??xxxx??xxxxxxx";

static const char *SetProgression_Signature = "\x48\x89\x5C\x24\x00\x48\x83\xEC\x20\x8B\xDA";
static const char *SetProgression_Mask = "xxxx??xxxx";

static const char *ShowNotification_Signature = "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xF9";
static const char *ShowNotification_Mask = "xxxx??xxxx??xxxxxxx";

void wd2_game_api_init(void) {
    wd2_log_info("Initializing game API...");

    HMODULE hGame = g_ctx.hGameModule;

    DWORD64 addr = wd2_pattern_scan(hGame, SpawnEntity_Signature, SpawnEntity_Mask);
    if (addr) {
        g_gameFuncs.SpawnEntityFromArchetypeHash = (SpawnEntity_t)addr;
        wd2_log_info("SpawnEntityFromArchetypeHash found at 0x%llX", addr);
    }

    addr = wd2_pattern_scan(hGame, GetEntityClassName_Signature, GetEntityClassName_Mask);
    if (addr) {
        g_gameFuncs.GetEntityClassName = (GetEntityClassName_t)addr;
        wd2_log_info("GetEntityClassName found at 0x%llX", addr);
    }

    addr = wd2_pattern_scan(hGame, RepairVehicle_Signature, RepairVehicle_Mask);
    if (addr) {
        g_gameFuncs.RepairVehicle = (RepairVehicle_t)addr;
        wd2_log_info("RepairVehicle found at 0x%llX", addr);
    }

    addr = wd2_pattern_scan(hGame, SetProgression_Signature, SetProgression_Mask);
    if (addr) {
        g_gameFuncs.SetProgression = (SetProgression_t)addr;
        wd2_log_info("SetProgression found at 0x%llX", addr);
    }

    addr = wd2_pattern_scan(hGame, ShowNotification_Signature, ShowNotification_Mask);
    if (addr) {
        g_gameFuncs.ShowNotification = (ShowNotification_t)addr;
        wd2_log_info("ShowNotification found at 0x%llX", addr);
    }

    wd2_log_info("Game API initialized");
}

void wd2_game_api_shutdown(void) {
    wd2_log_info("Game API shut down");
}

static int lua_GetPlayerPosition(lua_State *L) {
    float x = 0, y = 0, z = 0;
    if (g_gameFuncs.GetPlayerPosition) {
        g_gameFuncs.GetPlayerPosition(&x, &y, &z);
    }
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    return 3;
}

static int lua_GetLocalPlayerEntityId(lua_State *L) {
    int id = 0;
    if (g_gameFuncs.GetLocalPlayerEntityId) {
        id = g_gameFuncs.GetLocalPlayerEntityId();
    }
    lua_pushinteger(L, id);
    return 1;
}

static int lua_SetLocalPlayerNoclip(lua_State *L) {
    BOOL enable = (BOOL)lua_toboolean(L, 1);
    int result = 0;
    if (g_gameFuncs.SetLocalPlayerNoclip) {
        result = g_gameFuncs.SetLocalPlayerNoclip(enable);
    }
    lua_pushinteger(L, result);
    return 1;
}

static int lua_HasLocalPlayerNoclip(lua_State *L) {
    int result = 0;
    if (g_gameFuncs.HasLocalPlayerNoclip) {
        result = g_gameFuncs.HasLocalPlayerNoclip();
    }
    lua_pushboolean(L, result);
    return 1;
}

static int lua_SetLocalPlayerFreeCamera(lua_State *L) {
    BOOL enable = (BOOL)lua_toboolean(L, 1);
    int result = 0;
    if (g_gameFuncs.SetLocalPlayerFreeCamera) {
        result = g_gameFuncs.SetLocalPlayerFreeCamera(enable);
    }
    lua_pushinteger(L, result);
    return 1;
}

static int lua_HasLocalPlayerFreeCamera(lua_State *L) {
    int result = 0;
    if (g_gameFuncs.HasLocalPlayerFreeCamera) {
        result = g_gameFuncs.HasLocalPlayerFreeCamera();
    }
    lua_pushboolean(L, result);
    return 1;
}

static int lua_SetLocalPlayerGraphicKitModel(lua_State *L) {
    int modelId = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.SetLocalPlayerGraphicKitModel) {
        g_gameFuncs.SetLocalPlayerGraphicKitModel(modelId);
    }
    return 0;
}

static int lua_SpawnEntityFromArchetypeHash(lua_State *L) {
    int hash = (int)luaL_checkinteger(L, 1);
    float x = (float)luaL_optnumber(L, 2, 0.0);
    float y = (float)luaL_optnumber(L, 3, 0.0);
    float z = (float)luaL_optnumber(L, 4, 0.0);
    void *entity = NULL;
    if (g_gameFuncs.SpawnEntityFromArchetypeHash) {
        entity = g_gameFuncs.SpawnEntityFromArchetypeHash(hash, x, y, z);
    }
    lua_pushlightuserdata(L, entity);
    return 1;
}

static int lua_Teleport(lua_State *L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_checknumber(L, 3);
    if (g_gameFuncs.Teleport) {
        g_gameFuncs.Teleport(x, y, z);
    }
    return 0;
}

static int lua_CameraSetCustom(lua_State *L) {
    float px = (float)luaL_checknumber(L, 1);
    float py = (float)luaL_checknumber(L, 2);
    float pz = (float)luaL_checknumber(L, 3);
    float rx = (float)luaL_optnumber(L, 4, 0.0);
    float ry = (float)luaL_optnumber(L, 5, 0.0);
    float rz = (float)luaL_optnumber(L, 6, 0.0);
    float fov = (float)luaL_optnumber(L, 7, 60.0);
    if (g_gameFuncs.CameraSetCustom) {
        g_gameFuncs.CameraSetCustom(px, py, pz, rx, ry, rz, fov);
    }
    return 0;
}

static int lua_ShowNotification(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    wchar_t wtext[512];
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext, 512);
    if (g_gameFuncs.ShowNotification) {
        g_gameFuncs.ShowNotification(wtext);
    }
    return 0;
}

static int lua_SetProgression(lua_State *L) {
    int progress = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.SetProgression) {
        g_gameFuncs.SetProgression(progress);
    }
    return 0;
}

static int lua_RepairVehicle(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.RepairVehicle) {
        g_gameFuncs.RepairVehicle(entityId);
    }
    return 0;
}

static int lua_EnableVehicleEngine(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    BOOL enable = (BOOL)lua_toboolean(L, 2);
    if (g_gameFuncs.EnableVehicleEngine) {
        g_gameFuncs.EnableVehicleEngine(entityId, enable);
    }
    return 0;
}

static int lua_IsVehicleEngineEnabled(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int result = 0;
    if (g_gameFuncs.IsVehicleEngineEnabled) {
        result = g_gameFuncs.IsVehicleEngineEnabled(entityId);
    }
    lua_pushboolean(L, result);
    return 1;
}

static int lua_EnableVehicleSiren(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    BOOL enable = (BOOL)lua_toboolean(L, 2);
    if (g_gameFuncs.EnableVehicleSiren) {
        g_gameFuncs.EnableVehicleSiren(entityId, enable);
    }
    return 0;
}

static int lua_IsVehicleSirenEnabled(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int result = 0;
    if (g_gameFuncs.IsVehicleSirenEnabled) {
        result = g_gameFuncs.IsVehicleSirenEnabled(entityId);
    }
    lua_pushboolean(L, result);
    return 1;
}

static int lua_EnableVehicleEmergencyLight(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    BOOL enable = (BOOL)lua_toboolean(L, 2);
    if (g_gameFuncs.EnableVehicleEmergencyLight) {
        g_gameFuncs.EnableVehicleEmergencyLight(entityId, enable);
    }
    return 0;
}

static int lua_EnableVehicleIndicator(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    BOOL enable = (BOOL)lua_toboolean(L, 2);
    if (g_gameFuncs.EnableVehicleIndicator) {
        g_gameFuncs.EnableVehicleIndicator(entityId, enable);
    }
    return 0;
}

static int lua_EnableVehicleSpotLight(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    BOOL enable = (BOOL)lua_toboolean(L, 2);
    if (g_gameFuncs.EnableVehicleSpotLight) {
        g_gameFuncs.EnableVehicleSpotLight(entityId, enable);
    }
    return 0;
}

static int lua_EnableVehicleHighBeamLights(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    BOOL enable = (BOOL)lua_toboolean(L, 2);
    if (g_gameFuncs.EnableVehicleHighBeamLights) {
        g_gameFuncs.EnableVehicleHighBeamLights(entityId, enable);
    }
    return 0;
}

static int lua_EnableVehicleConvenienceLight(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    BOOL enable = (BOOL)lua_toboolean(L, 2);
    if (g_gameFuncs.EnableVehicleConvenienceLight) {
        g_gameFuncs.EnableVehicleConvenienceLight(entityId, enable);
    }
    return 0;
}

static int lua_SetVehicleLicensePlateText(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    const char *text = luaL_checkstring(L, 2);
    if (g_gameFuncs.SetVehicleLicensePlateText) {
        g_gameFuncs.SetVehicleLicensePlateText(entityId, text);
    }
    return 0;
}

static int lua_SetVehicleLightBehaviorFlag(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int flag = (int)luaL_checkinteger(L, 2);
    BOOL enable = (BOOL)lua_toboolean(L, 3);
    if (g_gameFuncs.SetVehicleLightBehaviorFlag) {
        g_gameFuncs.SetVehicleLightBehaviorFlag(entityId, flag, enable);
    }
    return 0;
}

static int lua_GetVehiclePartCount(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int count = 0;
    if (g_gameFuncs.GetVehiclePartCount) {
        count = g_gameFuncs.GetVehiclePartCount(entityId);
    }
    lua_pushinteger(L, count);
    return 1;
}

static int lua_GetVehiclePartHash(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int partIndex = (int)luaL_checkinteger(L, 2);
    int hash = 0;
    if (g_gameFuncs.GetVehiclePartHash) {
        hash = g_gameFuncs.GetVehiclePartHash(entityId, partIndex);
    }
    lua_pushinteger(L, hash);
    return 1;
}

static int lua_GetVehiclePartPosition(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int partIndex = (int)luaL_checkinteger(L, 2);
    float x = 0, y = 0, z = 0;
    if (g_gameFuncs.GetVehiclePartPosition) {
        g_gameFuncs.GetVehiclePartPosition(entityId, partIndex, &x, &y, &z);
    }
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    return 3;
}

static int lua_GetVehiclePartRotation(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int partIndex = (int)luaL_checkinteger(L, 2);
    float x = 0, y = 0, z = 0;
    if (g_gameFuncs.GetVehiclePartRotation) {
        g_gameFuncs.GetVehiclePartRotation(entityId, partIndex, &x, &y, &z);
    }
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    return 3;
}

static int lua_SetVehiclePartPosition(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int partIndex = (int)luaL_checkinteger(L, 2);
    float x = (float)luaL_checknumber(L, 3);
    float y = (float)luaL_checknumber(L, 4);
    float z = (float)luaL_checknumber(L, 5);
    if (g_gameFuncs.SetVehiclePartPosition) {
        g_gameFuncs.SetVehiclePartPosition(entityId, partIndex, x, y, z);
    }
    return 0;
}

static int lua_SetVehiclePartRotation(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int partIndex = (int)luaL_checkinteger(L, 2);
    float x = (float)luaL_checknumber(L, 3);
    float y = (float)luaL_checknumber(L, 4);
    float z = (float)luaL_checknumber(L, 5);
    if (g_gameFuncs.SetVehiclePartRotation) {
        g_gameFuncs.SetVehiclePartRotation(entityId, partIndex, x, y, z);
    }
    return 0;
}

static int lua_PutPlayerInVehicleDelayed(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    float delay = (float)luaL_optnumber(L, 2, 0.0f);
    if (g_gameFuncs.PutPlayerInVehicleDelayed) {
        g_gameFuncs.PutPlayerInVehicleDelayed(entityId, delay);
    }
    return 0;
}

static int lua_SetEntityGraphicKitModel(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int modelId = (int)luaL_checkinteger(L, 2);
    if (g_gameFuncs.SetEntityGraphicKitModel) {
        g_gameFuncs.SetEntityGraphicKitModel(entityId, modelId);
    }
    return 0;
}

static int lua_GetGraphicKitModelByEntityId(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int modelId = 0;
    if (g_gameFuncs.GetGraphicKitModelByEntityId) {
        modelId = g_gameFuncs.GetGraphicKitModelByEntityId(entityId);
    }
    lua_pushinteger(L, modelId);
    return 1;
}

static int lua_FelonyStartChase(lua_State *L) {
    int entityId = (int)luaL_optinteger(L, 1, 0);
    int result = 0;
    if (g_gameFuncs.FelonyStartChase) {
        result = g_gameFuncs.FelonyStartChase(entityId);
    }
    lua_pushinteger(L, result);
    return 1;
}

static int lua_ToggleFelonySystem(lua_State *L) {
    BOOL enable = (BOOL)lua_toboolean(L, 1);
    if (g_gameFuncs.ToggleFelonySystem) {
        g_gameFuncs.ToggleFelonySystem(enable);
    }
    return 0;
}

static int lua_ClearHeatLevel(lua_State *L) {
    if (g_gameFuncs.ClearHeatLevel) {
        g_gameFuncs.ClearHeatLevel();
    }
    return 0;
}

static int lua_SetHeatLevel(lua_State *L) {
    int level = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.SetHeatLevel) {
        g_gameFuncs.SetHeatLevel(level);
    }
    return 0;
}

static int lua_SetMinHeatLevel(lua_State *L) {
    int level = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.SetMinHeatLevel) {
        g_gameFuncs.SetMinHeatLevel(level);
    }
    return 0;
}

static int lua_SetSearchHeat(lua_State *L) {
    int heat = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.SetSearchHeat) {
        g_gameFuncs.SetSearchHeat(heat);
    }
    return 0;
}

static int lua_GetGlobalTrafficLightState(lua_State *L) {
    int state = 0;
    if (g_gameFuncs.GetGlobalTrafficLightState) {
        state = g_gameFuncs.GetGlobalTrafficLightState();
    }
    lua_pushinteger(L, state);
    return 1;
}

static int lua_SetGlobalTrafficLightState(lua_State *L) {
    int state = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.SetGlobalTrafficLightState) {
        g_gameFuncs.SetGlobalTrafficLightState(state);
    }
    return 0;
}

static int lua_ToggleWorldImpostor(lua_State *L) {
    BOOL enable = (BOOL)lua_toboolean(L, 1);
    if (g_gameFuncs.ToggleWorldImpostor) {
        g_gameFuncs.ToggleWorldImpostor(enable);
    }
    return 0;
}

static int lua_ToggleWorldSpawner(lua_State *L) {
    BOOL enable = (BOOL)lua_toboolean(L, 1);
    if (g_gameFuncs.ToggleWorldSpawner) {
        g_gameFuncs.ToggleWorldSpawner(enable);
    }
    return 0;
}

static int lua_IsWorldImpostorEnabled(lua_State *L) {
    int result = 0;
    if (g_gameFuncs.IsWorldImpostorEnabled) {
        result = g_gameFuncs.IsWorldImpostorEnabled();
    }
    lua_pushboolean(L, result);
    return 1;
}

static int lua_IsWorldSpawnerEnabled(lua_State *L) {
    int result = 0;
    if (g_gameFuncs.IsWorldSpawnerEnabled) {
        result = g_gameFuncs.IsWorldSpawnerEnabled();
    }
    lua_pushboolean(L, result);
    return 1;
}

static int lua_SetFrameRate(lua_State *L) {
    int fps = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.SetFrameRate) {
        g_gameFuncs.SetFrameRate(fps);
    }
    return 0;
}

static int lua_GetFrameRate(lua_State *L) {
    int fps = 0;
    if (g_gameFuncs.GetFrameRate) {
        fps = g_gameFuncs.GetFrameRate();
    }
    lua_pushinteger(L, fps);
    return 1;
}

static int lua_DetectGameVersion(lua_State *L) {
    if (g_gameFuncs.DetectGameVersion) {
        g_gameFuncs.DetectGameVersion();
    }
    lua_pushinteger(L, g_ctx.dwGameVersion);
    return 1;
}

static int lua_RefreshKeybinds(lua_State *L) {
    if (g_gameFuncs.RefreshKeybinds) {
        g_gameFuncs.RefreshKeybinds();
    }
    return 0;
}

static int lua_RefreshKeybind(lua_State *L) {
    int keyId = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.RefreshKeybind) {
        g_gameFuncs.RefreshKeybind(keyId);
    }
    return 0;
}

static int lua_PlayBroadcast(lua_State *L) {
    int broadcastId = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.PlayBroadcast) {
        g_gameFuncs.PlayBroadcast(broadcastId);
    }
    return 0;
}

static int lua_ResetBroadcastToDefault(lua_State *L) {
    if (g_gameFuncs.ResetBroadcastToDefault) {
        g_gameFuncs.ResetBroadcastToDefault();
    }
    return 0;
}

static int lua_KillSplashscreen(lua_State *L) {
    if (g_gameFuncs.KillSplashscreen) {
        g_gameFuncs.KillSplashscreen();
    }
    return 0;
}

static int lua_IsIngameUIEnabled(lua_State *L) {
    int result = 0;
    if (g_gameFuncs.IsIngameUIEnabled) {
        result = g_gameFuncs.IsIngameUIEnabled();
    }
    lua_pushboolean(L, result);
    return 1;
}

static int lua_ForceHackIngredient(lua_State *L) {
    int ingredientId = (int)luaL_checkinteger(L, 1);
    int count = (int)luaL_optinteger(L, 2, 1);
    if (g_gameFuncs.ForceHackIngredient) {
        g_gameFuncs.ForceHackIngredient(ingredientId, count);
    }
    return 0;
}

static int lua_ExecuteReward_V2(lua_State *L) {
    int rewardId = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.ExecuteReward_V2) {
        g_gameFuncs.ExecuteReward_V2(rewardId);
    }
    return 0;
}

static int lua_CDominoManager_GetInstance(lua_State *L) {
    void *instance = NULL;
    if (g_gameFuncs.CDominoManager_GetInstance) {
        instance = g_gameFuncs.CDominoManager_GetInstance();
    }
    lua_pushlightuserdata(L, instance);
    return 1;
}

static int lua_PlaySound(lua_State *L) {
    int soundId = (int)luaL_checkinteger(L, 1);
    if (g_gameFuncs.PlaySound) {
        g_gameFuncs.PlaySound(soundId);
    }
    return 0;
}

static int lua_GetAllEntities(lua_State *L) {
    int ids[1024];
    int count = 1024;
    if (g_gameFuncs.GetAllEntities) {
        g_gameFuncs.GetAllEntities(ids, &count);
    }
    lua_newtable(L);
    for (int i = 0; i < count; i++) {
        lua_pushinteger(L, ids[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

static int lua_GetEntitiesInRange(lua_State *L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_checknumber(L, 3);
    float range = (float)luaL_checknumber(L, 4);
    int ids[1024];
    int count = 1024;
    if (g_gameFuncs.GetEntitiesInRange) {
        g_gameFuncs.GetEntitiesInRange(x, y, z, range, ids, &count);
    }
    lua_newtable(L);
    for (int i = 0; i < count; i++) {
        lua_pushinteger(L, ids[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

static int lua_GetEntitiesWithComponent(lua_State *L) {
    int componentHash = (int)luaL_checkinteger(L, 1);
    int ids[1024];
    int count = 0;
    if (g_gameFuncs.GetEntitiesWithComponent) {
        count = g_gameFuncs.GetEntitiesWithComponent(componentHash, ids, 1024);
    }
    lua_newtable(L);
    for (int i = 0; i < count; i++) {
        lua_pushinteger(L, ids[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

static int lua_EntityHasComponent(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int componentHash = (int)luaL_checkinteger(L, 2);
    int result = 0;
    if (g_gameFuncs.EntityHasComponent) {
        result = g_gameFuncs.EntityHasComponent(entityId, componentHash);
    }
    lua_pushboolean(L, result);
    return 1;
}

static int lua_GetEntityClassName(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    const char *name = "unknown";
    if (g_gameFuncs.GetEntityClassName) {
        const char *result = g_gameFuncs.GetEntityClassName(entityId);
        if (result) name = result;
    }
    lua_pushstring(L, name);
    return 1;
}

static int lua_GetEntityComponents(lua_State *L) {
    int entityId = (int)luaL_checkinteger(L, 1);
    int hashes[64];
    int count = 64;
    if (g_gameFuncs.GetEntityComponents) {
        g_gameFuncs.GetEntityComponents(entityId, hashes, &count);
    }
    lua_newtable(L);
    for (int i = 0; i < count; i++) {
        lua_pushinteger(L, hashes[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

static int lua_RegisterCommand(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    if (!lua_isfunction(L, 2)) {
        luaL_error(L, "argument must be a function");
    }
    lua_getglobal(L, "__wd2_commands");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "__wd2_commands");
        lua_getglobal(L, "__wd2_commands");
    }
    lua_pushvalue(L, 2);
    lua_setfield(L, -2, name);
    lua_pop(L, 1);
    wd2_log_info("Registered command: %s", name);
    return 0;
}

void wd2_game_api_register_all(void) {
    lua_State *L = g_ctx.L;
    if (!L) return;

    lua_register(L, "RegisterCommand", lua_RegisterCommand);
    lua_register(L, "GetPlayerPosition", lua_GetPlayerPosition);
    lua_register(L, "GetLocalPlayerEntityId", lua_GetLocalPlayerEntityId);
    lua_register(L, "SetLocalPlayerNoclip", lua_SetLocalPlayerNoclip);
    lua_register(L, "HasLocalPlayerNoclip", lua_HasLocalPlayerNoclip);
    lua_register(L, "SetLocalPlayerFreeCamera", lua_SetLocalPlayerFreeCamera);
    lua_register(L, "HasLocalPlayerFreeCamera", lua_HasLocalPlayerFreeCamera);
    lua_register(L, "SetLocalPlayerGraphicKitModel", lua_SetLocalPlayerGraphicKitModel);
    lua_register(L, "SpawnEntityFromArchetypeHash", lua_SpawnEntityFromArchetypeHash);
    lua_register(L, "Teleport", lua_Teleport);
    lua_register(L, "CameraSetCustom", lua_CameraSetCustom);
    lua_register(L, "ShowNotification", lua_ShowNotification);
    lua_register(L, "SetProgression", lua_SetProgression);
    lua_register(L, "RepairVehicle", lua_RepairVehicle);
    lua_register(L, "EnableVehicleEngine", lua_EnableVehicleEngine);
    lua_register(L, "IsVehicleEngineEnabled", lua_IsVehicleEngineEnabled);
    lua_register(L, "EnableVehicleSiren", lua_EnableVehicleSiren);
    lua_register(L, "IsVehicleSirenEnabled", lua_IsVehicleSirenEnabled);
    lua_register(L, "EnableVehicleEmergencyLight", lua_EnableVehicleEmergencyLight);
    lua_register(L, "EnableVehicleIndicator", lua_EnableVehicleIndicator);
    lua_register(L, "EnableVehicleSpotLight", lua_EnableVehicleSpotLight);
    lua_register(L, "EnableVehicleHighBeamLights", lua_EnableVehicleHighBeamLights);
    lua_register(L, "EnableVehicleConvenienceLight", lua_EnableVehicleConvenienceLight);
    lua_register(L, "SetVehicleLicensePlateText", lua_SetVehicleLicensePlateText);
    lua_register(L, "SetVehicleLightBehaviorFlag", lua_SetVehicleLightBehaviorFlag);
    lua_register(L, "GetVehiclePartCount", lua_GetVehiclePartCount);
    lua_register(L, "GetVehiclePartHash", lua_GetVehiclePartHash);
    lua_register(L, "GetVehiclePartPosition", lua_GetVehiclePartPosition);
    lua_register(L, "GetVehiclePartRotation", lua_GetVehiclePartRotation);
    lua_register(L, "SetVehiclePartPosition", lua_SetVehiclePartPosition);
    lua_register(L, "SetVehiclePartRotation", lua_SetVehiclePartRotation);
    lua_register(L, "PutPlayerInVehicleDelayed", lua_PutPlayerInVehicleDelayed);
    lua_register(L, "SetEntityGraphicKitModel", lua_SetEntityGraphicKitModel);
    lua_register(L, "GetGraphicKitModelByEntityId", lua_GetGraphicKitModelByEntityId);
    lua_register(L, "FelonyStartChase", lua_FelonyStartChase);
    lua_register(L, "ToggleFelonySystem", lua_ToggleFelonySystem);
    lua_register(L, "ClearHeatLevel", lua_ClearHeatLevel);
    lua_register(L, "SetHeatLevel", lua_SetHeatLevel);
    lua_register(L, "SetMinHeatLevel", lua_SetMinHeatLevel);
    lua_register(L, "SetSearchHeat", lua_SetSearchHeat);
    lua_register(L, "GetGlobalTrafficLightState", lua_GetGlobalTrafficLightState);
    lua_register(L, "SetGlobalTrafficLightState", lua_SetGlobalTrafficLightState);
    lua_register(L, "ToggleWorldImpostor", lua_ToggleWorldImpostor);
    lua_register(L, "ToggleWorldSpawner", lua_ToggleWorldSpawner);
    lua_register(L, "IsWorldImpostorEnabled", lua_IsWorldImpostorEnabled);
    lua_register(L, "IsWorldSpawnerEnabled", lua_IsWorldSpawnerEnabled);
    lua_register(L, "SetFrameRate", lua_SetFrameRate);
    lua_register(L, "GetFrameRate", lua_GetFrameRate);
    lua_register(L, "DetectGameVersion", lua_DetectGameVersion);
    lua_register(L, "RefreshKeybinds", lua_RefreshKeybinds);
    lua_register(L, "RefreshKeybind", lua_RefreshKeybind);
    lua_register(L, "PlayBroadcast", lua_PlayBroadcast);
    lua_register(L, "ResetBroadcastToDefault", lua_ResetBroadcastToDefault);
    lua_register(L, "KillSplashscreen", lua_KillSplashscreen);
    lua_register(L, "IsIngameUIEnabled", lua_IsIngameUIEnabled);
    lua_register(L, "ForceHackIngredient", lua_ForceHackIngredient);
    lua_register(L, "ExecuteReward_V2", lua_ExecuteReward_V2);
    lua_register(L, "CDominoManager_GetInstance", lua_CDominoManager_GetInstance);
    lua_register(L, "PlaySound", lua_PlaySound);
    lua_register(L, "GetAllEntities", lua_GetAllEntities);
    lua_register(L, "GetEntitiesInRange", lua_GetEntitiesInRange);
    lua_register(L, "GetEntitiesWithComponent", lua_GetEntitiesWithComponent);
    lua_register(L, "EntityHasComponent", lua_EntityHasComponent);
    lua_register(L, "GetEntityClassName", lua_GetEntityClassName);
    lua_register(L, "GetEntityComponents", lua_GetEntityComponents);

    wd2_log_info("Registered %d Lua API functions", 64);
}
