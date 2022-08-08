#include "net_message.h"

#define PARSE_FUNC_NAME(type) parse_ ## type
#define PRINT_FUNC_NAME(type) print_ ## type
#define FREE_FUNC_NAME(type) free_ ## type
#define DECL_PARSE_FUNC(type) static void PARSE_FUNC_NAME(type)(NetSvcMessage* msg, BitStream* bits)
#define DECL_PRINT_FUNC(type) static void  PRINT_FUNC_NAME(type)(const NetSvcMessage* msg, FILE* fp)
#define DECL_FREE_FUNC(type) static void  FREE_FUNC_NAME(type)(NetSvcMessage* msg)

DECL_PARSE_FUNC(NetNop);
DECL_PRINT_FUNC(NetNop);
DECL_FREE_FUNC(NetNop);

DECL_PARSE_FUNC(NetDisconnect);
DECL_PRINT_FUNC(NetDisconnect);
DECL_FREE_FUNC(NetDisconnect);

DECL_PARSE_FUNC(NetFile);
DECL_PRINT_FUNC(NetFile);
DECL_FREE_FUNC(NetFile);

DECL_PARSE_FUNC(NetSplitScreenUser);
DECL_PRINT_FUNC(NetSplitScreenUser);
DECL_FREE_FUNC(NetSplitScreenUser);

DECL_PARSE_FUNC(NetTick);
DECL_PRINT_FUNC(NetTick);
DECL_FREE_FUNC(NetTick);

DECL_PARSE_FUNC(NetStringCmd);
DECL_PRINT_FUNC(NetStringCmd);
DECL_FREE_FUNC(NetStringCmd);

DECL_PARSE_FUNC(NetSetConVar);
DECL_PRINT_FUNC(NetSetConVar);
DECL_FREE_FUNC(NetSetConVar);

DECL_PARSE_FUNC(NetSignonState);
DECL_PRINT_FUNC(NetSignonState);
DECL_FREE_FUNC(NetSignonState);

DECL_PARSE_FUNC(SvcServerInfo);
DECL_PRINT_FUNC(SvcServerInfo);
DECL_FREE_FUNC(SvcServerInfo);

DECL_PARSE_FUNC(SvcSendTable);
DECL_PRINT_FUNC(SvcSendTable);
DECL_FREE_FUNC(SvcSendTable);

DECL_PARSE_FUNC(SvcClassInfo);
DECL_PRINT_FUNC(SvcClassInfo);
DECL_FREE_FUNC(SvcClassInfo);

DECL_PARSE_FUNC(SvcSetPause);
DECL_PRINT_FUNC(SvcSetPause);
DECL_FREE_FUNC(SvcSetPause);

DECL_PARSE_FUNC(SvcCreateStringTable);
DECL_PRINT_FUNC(SvcCreateStringTable);
DECL_FREE_FUNC(SvcCreateStringTable);

DECL_PARSE_FUNC(SvcUpdateStringTable);
DECL_PRINT_FUNC(SvcUpdateStringTable);
DECL_FREE_FUNC(SvcUpdateStringTable);

DECL_PARSE_FUNC(SvcVoiceInit);
DECL_PRINT_FUNC(SvcVoiceInit);
DECL_FREE_FUNC(SvcVoiceInit);

DECL_PARSE_FUNC(SvcVoiceData);
DECL_PRINT_FUNC(SvcVoiceData);
DECL_FREE_FUNC(SvcVoiceData);

DECL_PARSE_FUNC(SvcPrint);
DECL_PRINT_FUNC(SvcPrint);
DECL_FREE_FUNC(SvcPrint);

DECL_PARSE_FUNC(SvcSounds);
DECL_PRINT_FUNC(SvcSounds);
DECL_FREE_FUNC(SvcSounds);

DECL_PARSE_FUNC(SvcSetView);
DECL_PRINT_FUNC(SvcSetView);
DECL_FREE_FUNC(SvcSetView);

DECL_PARSE_FUNC(SvcFixAngle);
DECL_PRINT_FUNC(SvcFixAngle);
DECL_FREE_FUNC(SvcFixAngle);

DECL_PARSE_FUNC(SvcCrosshairAngle);
DECL_PRINT_FUNC(SvcCrosshairAngle);
DECL_FREE_FUNC(SvcCrosshairAngle);

DECL_PARSE_FUNC(SvcBspDecal);
DECL_PRINT_FUNC(SvcBspDecal);
DECL_FREE_FUNC(SvcBspDecal);

DECL_PARSE_FUNC(SvcSplitScreen);
DECL_PRINT_FUNC(SvcSplitScreen);
DECL_FREE_FUNC(SvcSplitScreen);

DECL_PARSE_FUNC(SvcUserMessage);
DECL_PRINT_FUNC(SvcUserMessage);
DECL_FREE_FUNC(SvcUserMessage);

DECL_PARSE_FUNC(SvcEntityMessage);
DECL_PRINT_FUNC(SvcEntityMessage);
DECL_FREE_FUNC(SvcEntityMessage);

DECL_PARSE_FUNC(SvcGameEvent);
DECL_PRINT_FUNC(SvcGameEvent);
DECL_FREE_FUNC(SvcGameEvent);

DECL_PARSE_FUNC(SvcPacketEntities);
DECL_PRINT_FUNC(SvcPacketEntities);
DECL_FREE_FUNC(SvcPacketEntities);

DECL_PARSE_FUNC(SvcTempEntities);
DECL_PRINT_FUNC(SvcTempEntities);
DECL_FREE_FUNC(SvcTempEntities);

DECL_PARSE_FUNC(SvcPrefetch);
DECL_PRINT_FUNC(SvcPrefetch);
DECL_FREE_FUNC(SvcPrefetch);

DECL_PARSE_FUNC(SvcMenu);
DECL_PRINT_FUNC(SvcMenu);
DECL_FREE_FUNC(SvcMenu);

DECL_PARSE_FUNC(SvcGameEventList);
DECL_PRINT_FUNC(SvcGameEventList);
DECL_FREE_FUNC(SvcGameEventList);

DECL_PARSE_FUNC(SvcGetCvarValue);
DECL_PRINT_FUNC(SvcGetCvarValue);
DECL_FREE_FUNC(SvcGetCvarValue);

DECL_PARSE_FUNC(SvcCmdKeyValues);
DECL_PRINT_FUNC(SvcCmdKeyValues);
DECL_FREE_FUNC(SvcCmdKeyValues);

DECL_PARSE_FUNC(SvcPaintmapData);
DECL_PRINT_FUNC(SvcPaintmapData);
DECL_FREE_FUNC(SvcPaintmapData);

DECL_PARSE_FUNC(Invalid1) {}
DECL_PRINT_FUNC(Invalid1) {}
DECL_FREE_FUNC(Invalid1) {}

DECL_PARSE_FUNC(Invalid2) {}
DECL_PRINT_FUNC(Invalid2) {}
DECL_FREE_FUNC(Invalid2) {}

#define DECL_PARSE_FUNCS_IN_ARRAY(x) PARSE_FUNC_NAME(x),
#define DECL_PRINT_FUNCS_IN_ARRAY(x) PRINT_FUNC_NAME(x),
#define DECL_FREE_FUNCS_IN_ARRAY(x) FREE_FUNC_NAME(x),

ParseNetSvcMsgFunc parse_oe_net_msg_func[OE_MSG_COUNT] = {
    MACRO_OE_MESSAGES(DECL_PARSE_FUNCS_IN_ARRAY)
};
PrintNetSvcMsgFunc print_oe_net_msg_func[OE_MSG_COUNT] = {
    MACRO_OE_MESSAGES(DECL_PRINT_FUNCS_IN_ARRAY)
};
FreeNetSvcMsgFunc free_oe_net_msg_func[OE_MSG_COUNT] = {
    MACRO_OE_MESSAGES(DECL_FREE_FUNCS_IN_ARRAY)
};

ParseNetSvcMsgFunc parse_ne_net_msg_func[NE_MSG_COUNT] = {
    MACRO_NE_MESSAGES(DECL_PARSE_FUNCS_IN_ARRAY)
};
PrintNetSvcMsgFunc print_ne_net_msg_func[NE_MSG_COUNT] = {
    MACRO_NE_MESSAGES(DECL_PRINT_FUNCS_IN_ARRAY)
};
FreeNetSvcMsgFunc free_ne_net_msg_func[NE_MSG_COUNT] = {
    MACRO_NE_MESSAGES(DECL_FREE_FUNCS_IN_ARRAY)
};