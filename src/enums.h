#include <cstdint>

#ifndef DOGWATER_ENUMS_H
#define DOGWATER_ENUMS_H

enum class SendOption : uint8_t {
    Unreliable = 0,
    Reliable,
    Hello = 8,
    Disconnect,
    Acknowledge,
    Ping = 12
};

enum class DisconnectReason : uint8_t {
    ExitGame,
    GameFull,
    GameStarted,
    GameNotFound,
    IncorrectVersion = 5,
    Banned,
    Kicked,
    Custom,
    InvalidName,
    Hacking,
    NotAuthorized,
    Destroy = 16,
    Error,
    IncorrectGame,
    ServerRequest,
    ServerFull,
    InternalPlayerMissing = 100,
    InternalNonceFailure,
    InternalConnectionToken,
    PlatformLock,
    LobbyInactivity,
    MatchmakerInactivity,
    InvalidGameOptions,
    NoServersAvailable,
    QuickmatchDisabled,
    TooManyGames,
    QuickchatLock,
    MatchmakerFull,
    Sanctions,
    ServerError,
    SelfPlatformLock,
    IntentionalLeaving = 208,
    FocusLostBackground = 207,
    FocusLost = 209,
    NewConnection,
    PlatformParentalControlsBlock,
    PlatformUserBlock,
    None = 255
};

enum class Platform : uint8_t {
    Unknown,
    StandaloneEpicPC,
    StandaloneSteamPC,
    StandaloneMac,
    StandaloneWin10,
    StandaloneItch,
    IPhone,
    Android,
    Switch,
    Xbox,
    Playstation
};

enum class ReactorMessageTag : uint8_t {
    Handshake,
    ModDeclaration,
    PluginDeclaration
};

enum class ModPluginSide : uint8_t {
    ClientSide,
    Both
};

#endif //DOGWATER_ENUMS_H
