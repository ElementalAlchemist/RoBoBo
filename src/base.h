#pragma once
#include "main.h"
#include <dlfcn.h>

enum LoadResult { LOAD_SUCCESS, LOAD_ALREADYLOADED, LOAD_ERROR, LOAD_INCOMPATIBLE, LOAD_NODEPENDS, LOAD_FAILURE };
class Base;

#include "socket.h"
#include "protocol.h"
#include "module.h"

// The spawn function needs a typedef so that it can be called after a dlsym.
// This typedef is used to cast the result of dlsym so that it can be used. (see Base::loadModule)
typedef void* (module_spawn_t)(std::string, std::map<std::string, std::string>, std::string, unsigned short, Base*);

class Base {
	public:
		/** Constructor for the Base class
		 * This function serves as the constructor of Base, accepting parameters for various things
		 * it needs to be able to start the bot.
		 * @param working The working directory of the bot (i.e. the relative path to the executable
		 * from where the bot was started)
		 * @param config The relative directory of the config file from the working directory
		 * @param configFileName The name of the configuration file
		 * @param debug The debug level at which the bot was started
		 * @param log Whether the bot is to generate logs; sent to protocol modules
		 */
		Base(std::string working, std::string config, std::string configFileName, unsigned short debug, bool log);
		
		/** Configuration file reader
		 * Executed as part of the bot's startup routine, this opens the config file and reads it,
		 * storing it in various variables throughout the Base class to be read as needed.
		 */
		void readConfiguration();
		
		/** Startup module loader
		 * Executed as part of the bot's startup routine, this loads all of the modules specified in
		 * the configuration to be loaded on the bot's startup.
		 */
		void loadModules();
		
		/** Startup module load completion
		 * Executed from loadModules, this function is called for each module after all of the modules
		 * have been loaded on startup and returns true if everything went well.
		 * @param modName The name of the module being checked
		 * @param modptr A pointer to the module class for the module in question
		 */
		bool completeStartupLoad(std::string modName, Module* modptr);
		
		/** Server connecter
		 * Executed as part of the bot's startup routine, this loads all of the needed protocol
		 * modules and connects to the servers specified in the configuration to be connected on
		 * startup.
		 */
		void connectServers();
		
		/** Server check thread
		 * This function serves as a thread running continuously once the bot is started up, and
		 * it continues to run until it determines that the bot should shut down.  Its purpose is
		 * to detect servers that have been disconnected and reconnect them if appropriate.
		 */
		void checkServers();
		
		/** Destroyer of everything
		 * This function runs after checkServers exits, and its purpose is to safely unload everything.
		 */
		void unloadEverything();
		
		/** Module loader
		 * This function loads modules and verifies that all is well with them.  As part of the loading
		 * process, a module may signal to this function that it cannot be loaded.
		 * @param modName The name of the module to load
		 * @return A code indicating if the load was successful or the cause of error
		 */
		LoadResult loadModule(std::string modName);
		
		/** Module unloader
		 * This function unloads modules.
		 * @param modName The name of the module to unload
		 * @param wasLoaded A parameter used by the startup loadModules function to tell this function not
		 * to call the module unload hooks
		 */
		void unloadModule(std::string modName, bool wasLoaded);
		
		/** Server connecter
		 * This function loads a protocol module if necessary and connects to the specified server.
		 * @param server The name of the server to connect
		 */
		void connectServer(std::string server);
		
		/** Server disconnecter
		 * This function disconnects from the specified server and, if necessary, unloads the appropriate
		 * protocol module.
		 * @param server The name of the server to disconnect
		 */
		void disconnectServer(std::string server);
		
		/** Socket module loader
		 * This function loads a socket module requested by a protocol module and returns the socket.
		 * @param sockettype The name of the type of socket to load
		 */
		Socket* loadSocket(std::string sockettype);
		
		/** Socket module unloader
		 * This function unloads a socket module when the protocol module is done with it (e.g. if the server
		 * is disconnecting or some such)
		 * @param sockettype The name of the type of socket being unloaded
		 * @param socketptr A pointer to the socket class needing removed
		 */
		void unloadSocket(std::string sockettype, Socket* socketptr);
		
		
		/** Channel message hook thread
		 * This function makes channel message hook calls "nonblocking" by calling the function that actually makes
		 * the hook calls in a new thread.  It is called by a protocol module when the bot receives a message sent
		 * to a channel.
		 * @param server The name of the server on which the channel message was received
		 * @param client The identifier of the client that received the message
		 * @param channel The name of the channel through which the message was received
		 * @param status The status character to which the message was sent, e.g. '@' if the message was sent to @#channel;
		 * when none, it is just a blank space (' ')
		 * @param nick The nick of the user who sent the message
		 * @param message The message received by the bot
		 * @see callChanMsgHooks
		 */
		void modChanMsgHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		
		/** User message hook thread
		 * This function makes user message hook calls "nonblocking" by calling the function that actually makes
		 * the hook calls in a new thread.  It is called by a protocol module when the bot receives a message
		 * sent directly from a user.
		 * @param server The name of the server on which the message was received
		 * @param client The identifier of the client that received the message
		 * @param nick The nick of the user who sent the message
		 * @param message The message received by the bot
		 * @see callUserMsgHooks
		 */
		void modUserMsgHook(std::string server, std::string client, std::string nick, std::string message);
		
		/** Channel notice hook thread
		 * This function makes channel notice hook calls "nonblocking" by calling the function that actually makes
		 * the hook calls in a new thread.  It is called by a protocol module when the bot receives a notice sent
		 * to a channel.
		 * @param server The name of the server on which the notice was received
		 * @param client The identifier of the client that received the notice
		 * @param channel The name of the channel through which the notice was received
		 * @param status The status character to which the notice was sent, e.g. '@' if the notice was sent to @#channel;
		 * when none, it is just a blank space (' ')
		 * @param nick The nick of the user who sent the notice
		 * @param message The contents of the notice
		 * @see callChanNoticeHooks
		 */
		void modChanNoticeHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		
		/** User notice hook thread
		 * This function makes user notice hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when the bot receives a notice sent directly
		 * from a user.
		 * @param server The name of the server on which the notice was received
		 * @param client The identifier of the client that received the notice
		 * @param nick The nick of the user who sent the notice
		 * @param message The contents of the notice
		 * @see callUserNoticeHooks
		 */
		void modUserNoticeHook(std::string server, std::string client, std::string nick, std::string message);
		
		/** Channel CTCP hook thread
		 * This function makes channel CTCP hook calls "nonblocking" by calling the function that actually makes
		 * the hook calls in a new thread.  It is called by a protocol module when the bot receives a CTCP request
		 * sent to a channel.
		 * @param server The name of the server on which the CTCP request was received
		 * @param client The identifier of the client that received the CTCP request
		 * @param channel The name of the channel through which the CTCP request was received
		 * @param status The status character to which the CTCP request was sent, e.g. '@' if the notice was sent to
		 * @#channel; when none, it is just a blank space (' ')
		 * @param nick The nick of the user who sent the CTCP request
		 * @param ctcp The type of CTCP request, e.g. "PING" "VERSION" etc.
		 * @param data Any parameters associated with the CTCP request
		 * @see callChanCTCPHooks
		 */
		void modChanCTCPHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		
		/** User CTCP hook thread
		 * This function makes user CTCP hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when the bot receives a CTCP request sent
		 * directly from a user.
		 * @param server The name of the server on which the CTCP request was received
		 * @param client The identifier of the client that received the CTCP request
		 * @param nick The nick of the user who sent the CTCP request
		 * @param ctcp The type of CTCP request, e.g. "PING" "VERSION" etc.
		 * @param data Any parameters associated with the CTCP request
		 * @see callUserCTCPHooks
		 */
		void modUserCTCPHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		
		/** Channel CTCP reply hook thread
		 * This function makes channel CTCP reply hook calls "nonblocking" by calling the function that actually
		 * makes the hook calls in a new thread.  It is called by a protocol module when the bot receives a CTCP
		 * reply sent to a channel.
		 * @param server The name of the server on which the CTCP reply was received
		 * @param client The identifier of the client that received the CTCP reply
		 * @param channel The name of the channel through which the CTCP reply was received
		 * @param status The status character to which the CTCP reply was sent, e.g. '@' if the CTCP reply was sent
		 * to @#channel; when none, it is just a blank space (' ')
		 * @param nick The nick of the user who sent the CTCP reply
		 * @param ctcp The type of CTCP reply, e.g. "PING" "VERSION" etc.
		 * @param data Any further data assocciated with the CTCP reply
		 * @see callChanCTCPReplyHooks
		 */
		void modChanCTCPReplyHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		
		/** User CTCP reply hook thread
		 * This function makes user CTCP reply hook calls "nonblocking" by calling the function that actually makes
		 * the hook calls in a new thread.  It is called by a protocol module when the bot receives a CTCP reply sent
		 * directly from a user.
		 * @param server The name of the server on which the CTCP reply was received
		 * @param client The identifier of the client that received the CTCP reply
		 * @param nick The nick of the user who sent the CTCP reply
		 * @param ctcp The type of CTCP reply, e.g. "PING" "VERSION" etc.
		 * @param data Any further data associated with the CTCP reply
		 * @see callUserCTCPReplyHooks
		 */
		void modUserCTCPReplyHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		
		/** Channel mode hook thread
		 * This function makes channel mode hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when the bot receives notice from the server
		 * of a change of modes in a channel.
		 * @param server The name of the server on which channel modes were changed
		 * @param client The identifier of the client that received the mode change notice
		 * @param channel The channel that had its modes changed
		 * @param setter The nick of the user who changed the modes
		 * @param add Whether the mode was set or unset; true if the mode is being set
		 * @param mode The long name of the mode being set
		 * @param param The mode parameter, or empty string if there is none
		 * @see callChanModeHooks
		 */
		void modChanModeHook(std::string server, std::string client, std::string channel, std::string setter, bool add, std::string mode, std::string param);
		
		/** User mode hook thread
		 * This function makes user mode hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when the bot receives notice from the server of a
		 * change of user modes.
		 * @param server The name of the server on which user modes were changed
		 * @param client The identifier of the client whose modes were changed
		 * @param add Whether the mode was set or unset; true if the mode is being set
		 * @param mode The long name of the mode being set
		 * @see callUserModeHooks
		 */
		void modUserModeHook(std::string server, std::string client, bool add, std::string mode);
		
		/** User SNOMask hook thread
		 * This function makes user SNOMask hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when the bot receives notice from the server
		 * of changes in a client's server notice masks.
		 * @param server The name of the server on which server notice masks were changed
		 * @param client The identifier of the client whose SNOMask was changed
		 * @param add Whether the SNOmask in question was added or removed; true if it was added
		 * @param snomask The snomask that was added or removed; is a string in case some server uses long names instead of mode-like characters
		 * @see callUserSNOMaskHooks
		 */
		void modUserSNOMaskHook(std::string server, std::string client, bool add, std::string snomask);
		
		/** Channel topic hook thread
		 * This function makes channel topic hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when the topic of a channel is changed.
		 * @param server The name of the server on which the topic was changed
		 * @param client The identifier of the client receiving this notice
		 * @param channel The channel whose topic was changed
		 * @param setter The nick of the user who changed the topic
		 * @param topic The new channel topic
		 * @see callChanTopicHooks
		 */
		void modChanTopicHook(std::string server, std::string client, std::string channel, std::string setter, std::string topic);
		
		/** Channel join hook thread
		 * This function makes channel join hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when someone joins a channel.
		 * @param server The name of the server on which a user joined a channel
		 * @param client The identifier of the client receiving the notice of joining
		 * @param channel The channel that was joined
		 * @param nick The nick of the user who joined
		 * @see callChanJoinHooks
		 */
		void modChanJoinHook(std::string server, std::string client, std::string channel, std::string nick);
		
		/** Channel part hook thread
		 * This function makes channel part hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when someone parts a channel.
		 * @param server The server on which a user parted a channel
		 * @param client The identifier of the client receiving the notice of parting
		 * @param channel The channel that was parted
		 * @param nick The nick of the user who parted
		 * @param reason The part reason given by the user
		 * @see callChanPartHooks
		 */
		void modChanPartHook(std::string server, std::string client, std::string channel, std::string nick, std::string reason);
		
		/** User connect hook thread
		 * This function makes user connect hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when someone connects to the network.
		 * @param server The server on the network to which the user connected
		 * @param nick The nick of the user who connected
		 * @see callUserConnectHooks
		 */
		void modUserConnectHook(std::string server, std::string nick);
		
		/** User quit hook thread
		 * This function makes user quit hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when someone quits from the network.
		 * @param server The server on the network from which the user quit
		 * @param client The identifier of the client receiving the notice of quitting
		 * @param nick The nick of the user who quit
		 * @param reason The quit reason given
		 * @see callUserQuitHooks
		 */
		void modUserQuitHook(std::string server, std::string client, std::string nick, std::string reason);
		
		/** User nick hook thread
		 * This function makes user quit hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when someone changes their nick.
		 * @param server The server on which the user changed their nick
		 * @param client The identifier of the client receiving the nick change notice
		 * @param oldNick The original nick of the user
		 * @param newNick The new nick of the user
		 * @see callUserNickHooks
		 */
		void modUserNickHook(std::string server, std::string client, std::string oldNick, std::string newNick);
		
		/** Numeric hook thread
		 * This function makes numeric hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when a numeric is received from the server.
		 * @param server The server that sent the numeric
		 * @param client The identifier of the client receiving the numeric
		 * @param numeric The numeric
		 * @param data A vector of tokens passed to the client with the numeric
		 * @see callNumericHooks
		 */
		void modNumericHook(std::string server, std::string client, std::string numeric, std::vector<std::string> data);
		
		/** Oper hook thread
		 * This function makes oper hook calls "nonblocking" by calling the function that actually makes the hook calls
		 * in a new thread.  It is called by a protocol module when a user is opered.
		 * @param server The server on which the user was opered
		 * @param nick The nick of the opered user
		 * @param operType The type of oper the user now is
		 * @see callOperHooks
		 */
		void modOperHook(std::string server, std::string nick, std::string operType);
		
		/** Server notice hook thread
		 * This function makes SNOtice hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when a local client receives a server notice.
		 * @param server The server that sent the notice
		 * @param snotype The SNOmask the message was sent under
		 * @param message The contents of the server notice
		 * @see callSNoticeHooks
		 */
		void modSNoticeHook(std::string server, std::string snotype, std::string message);
		
		/** Metadata hook thread
		 * This function makes metadata hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when a user or channel has its metadata changed.
		 * @param server The server on which the metadata was changed
		 * @param target The user or channel whose metadata was changed
		 * @param dataKey The key of the metadata that was changed
		 * @param dataValue The new value of the metadata
		 * @see callMetadataHooks
		 */
		void modMetadataHook(std::string server, std::string target, std::string dataKey, std::string dataValue);
		
		/** x:line add hook thread
		 * This function makes x:line add hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when an x:line is set.
		 * @param server The server on which the x:line was set
		 * @param lineType The type of x:line set ("G" "Z" "SHUN" etc.)
		 * @param mask The mask the x:line was set on
		 * @param setter The user who set the x:line
		 * @param expiry The time the x:line expires
		 * @param reason The reason given for the x:line
		 * @see callXLineAddHooks
		 */
		void modXLineAddHook(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason);
		
		/** x:line remove hook thread
		 * This function makes x:line removal hook calls "nonblocking" by calling the function that actually makes
		 * the hook calls in a new thread.  It is called by a protocol module when an x:line is unset or expires.
		 * @param server The server on which the x:line was unset
		 * @param lineType The type of x:line that was unset
		 * @param mask The mask the x:line was originally set on
		 * @see callXLineRemoveHooks
		 */
		void modXLineRemoveHook(std::string server, std::string lineType, std::string mask);
		
		/** Server connect hook thread
		 * This function makes server connect hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when a new server connects to the network.
		 * @param server The server in the network to which the new server was connected
		 * @param newServerName The name of the server being introduced
		 * @see callServerConnectHooks
		 */
		void modServerConnectHook(std::string server, std::string newServerName);
		
		/** Server quit hook thread
		 * This function makes server quit hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when a server quits from the network.
		 * @param server The server in the network to which the bot is connected
		 * @param quitServerName The name of the server that quit
		 * @param reason The quit reason provided by the leaving server
		 * @see callServerQuitHooks
		 */
		void modServerQuitHook(std::string server, std::string quitServerName, std::string reason);
		
		/** Other data hook thread
		 * This function makes other data hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when any line is received that cannot be handled
		 * by other hooks.
		 * @param server The server from which this line was received
		 * @param client The identifier for the client who received this data
		 * @param lineTokens The line data split into IRC tokens
		 * @see callOtherDataHooks
		 */
		void modOtherDataHook(std::string server, std::string client, std::vector<std::string> lineTokens);
		
		/** Outgoing channel message hook thread
		 * This function makes outgoing channel message hook calls "nonblocking" by calling the function that actually
		 * makes the hook calls in a new thread.  It is called by a protocol module when it is about to send a message
		 * to a channel.
		 * @param server The server to which the message will be sent
		 * @param client The identifier of the client from which the message will be sent
		 * @param channel The channel to which the message will be sent
		 * @param status The status char to which the message will be sent, e.g. '@' if the message will be sent to
		 * @#channel; if none, it is a blank space (' ')
		 * @param message The message to be sent
		 * @see callChanMsgOutHooks
		 */
		void modChanMsgOutHook(std::string server, std::string client, std::string channel, char status, std::string message);
		
		/** Sent channel message hook thread
		 * This function makes sent channel message hook calls "nonblocking" by calling the function that actually makes
		 * the hook calls in a new thread.  It is called by a protocol module when it has just sent a message to a channel.
		 * @param server The server to which the message was sent
		 * @param client The identifier of the client from which the message was sent
		 * @param channel The channel to which the message was sent
		 * @param status The status char to which the message was sent, e.g. '@' if the message was sent to @#channel; if
		 * none, it is a blank space (' ')
		 * @param message The message that was sent
		 * @see callChanMsgSendHooks
		 */
		void modChanMsgSendHook(std::string server, std::string client, std::string channel, char status, std::string message);
		
		/** Outgoing user message hook thread
		 * This function makes outgoing user message hook calls "nonblocking" by calling the function that actually makes
		 * the hook calls in a new thread.  It is called by a protocol module when it is about to send a message to another
		 * user.
		 * @param server The server to which the message will be sent
		 * @param client The identifier of the client from which the message will be sent
		 * @param nick The nick of the user to whom the message will be sent
		 * @param message The message to be sent
		 * @see callUserMsgOutHooks
		 */
		void modUserMsgOutHook(std::string server, std::string client, std::string nick, std::string message);
		
		/** Sent user message hook thread
		 * This function makes sent user message hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when it has just sent a message to another user.
		 * @param server The server to which the message was sent
		 * @param client The identifier of the client from which the message was sent
		 * @param nick The nick of the user to whom the message was sent
		 * @param message The message that was sent
		 * @see callUserMsgSendHooks
		 */
		void modUserMsgSendHook(std::string server, std::string client, std::string nick, std::string message);
		
		/** Outgoing channel notice hook thread
		 * This function makes outgoing channel notice hook calls "nonblocking" by calling the function that actually makes
		 * the hook calls in a new thread.  It is called by a protocol module when it is about to send a notice to a channel.
		 * @param server The server to which the notice will be sent
		 * @param client The identifier of the client from which the notice will be sent
		 * @param channel The channel to which the notice will be sent
		 * @param status The status char to which the notice will be sent, e.g. '@' if the notice is to be sent to @#channel; if
		 * none, it is a blank space (' ')
		 * @param message The contents of the notice to be sent
		 * @see callChanNoticeOutHooks
		 */
		void modChanNoticeOutHook(std::string server, std::string client, std::string channel, char status, std::string message);
		
		/** Sent channel notice hook thread
		 * This function makes sent channel notice hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when it has just sent a notice to a channel.
		 * @param server The server to which the notice was sent
		 * @param client The identifier of the client from which the notice was sent
		 * @param channel The channel to which the notice was sent
		 * @param status The status char to which the notice was sent, e.g. '@' if the notice was sent to @#channel; if none, it
		 * is a blank space (' ')
		 * @param message The contents of the notice that was sent
		 * @see callChanNoticeSendHooks
		 */
		void modChanNoticeSendHook(std::string server, std::string client, std::string channel, char status, std::string message);
		
		/** Outgoing user notice hook thread
		 * This function makes outgoing user notice hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when it is about to send a notice to another user.
		 * @param server The server to which the notice is to be sent
		 * @param client The identifier of the client from which the notice is to be sent
		 * @param nick The nick of the user to whom the message will be sent
		 * @param message The contents of the notice to be sent
		 * @see callUserNoticeOutHooks
		 */
		void modUserNoticeOutHook(std::string server, std::string client, std::string nick, std::string message);
		
		/** Sent user notice hook thread
		 * This function makes sent user notice hook calls "nonblocking" by calling the function that actually makes the hook calls
		 * in a new thread.  It is called by a protocol module when it has sent a notice to another user.
		 * @param server The server to which the notice was sent
		 * @param client The identifier of the client from which the notice was sent
		 * @param nick The nick of the user to whom the notice was sent
		 * @param message The contents of the notice that was sent
		 * @see callUserNoticeSendHooks
		 */
		void modUserNoticeSendHook(std::string server, std::string client, std::string nick, std::string message);
		
		/** Outgoing channel CTCP hook thread
		 * This function makes outgoing channel CTCP hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when it is about to send a CTCP request to a channel.
		 * @param server The server to which the CTCP request is to be sent
		 * @param client The identifier of the client to send the CTCP request
		 * @param channel The channel to which the CTCP request is to be sent
		 * @param status The status char to which the CTCP request is to be sent, e.g. '@' if the CTCP request is to be sent to
		 * @#channel; if none, it is a blank space (' ')
		 * @param ctcp The type of CTCP request, e.g. "PING" "VERSION" etc.
		 * @param params Any parameters associated with the CTCP request
		 * @see callChanCTCPOutHooks
		 */
		void modChanCTCPOutHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		
		/** Sent channel CTCP hook thread
		 * This function makes sent channel CTCP hook calls "nonblocking" by calling the function that actually makes the hook calls
		 * in a new thread.  It is called by a protocol module when it has sent a CTCP request to a channel.
		 * @param server The server to which the CTCP request was sent
		 * @param client The identifier of the client that sent the CTCP request
		 * @param channel The channel to which the CTCP request was sent
		 * @param status The status char to which the CTCP request was sent, e.g. '@' if the CTCP request was sent to @#channel;
		 * if none, it is a blank space (' ')
		 * @param ctcp The type of CTCP request, e.g. "PING" "VERSION" etc.
		 * @param params Any parameters associated with the CTCP request
		 * @see callChanCTCPSendHooks
		 */
		void modChanCTCPSendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		
		/** Outgoing user CTCP hook thread
		 * This function makes outgoing user CTCP hook calls "nonblocking" by calling the function that actually makes the hook calls
		 * in a new thread.  It is called by a protocol module when it is about to send a CTCP request to another user.
		 * @param server The server to which the CTCP request is to be sent
		 * @param client The identifier of the client to send the CTCP request
		 * @param nick The nick of the user to whom the CTCP request is to be sent
		 * @param ctcp The type of CTCP request, e.g. "PING" "VERSION" etc.
		 * @param params Any parameters associated with the CTCP request
		 * @see callUserCTCPOutHooks
		 */
		void modUserCTCPOutHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		/** Sent user CTCP hook thread
		 * This function makes sent user CTCP hook calls "nonblocking" by calling the function that actually makes the hook calls in
		 * a new thread.  It is called by a protocol module when it has sent a CTCP request to another user.
		 * @param server The server to which the CTCP request was sent
		 * @param client The identifier of the client from which the CTCP request was sent
		 * @param nick The nick of the user to whom the CTCP request was sent
		 * @param ctcp The type of CTCP request, e.g. "PING" "VERSION" etc.
		 * @param params Any parameters associated with the CTCP request
		 * @see callUserCTCPSendHooks
		 */
		void modUserCTCPSendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		/** Outgoing channel CTCP reply hook thread
		 * This function makes outgoing channel CTCP reply hook calls "nonblocking" by calling the function that actually makes the
		 * hook calls in a new thread.  It is called by a protocol module when it is about to send a CTCP reply to a channel.
		 * @param server The server to which the CTCP reply is to be sent
		 * @param client The identifier of the client from which the CTCP reply is to be sent
		 * @param channel The channel to which the CTCP reply is to be sent
		 * @param status The status char to which the CTCP reply is being sent, e.g. '@' if the CTCP reply is being sent to @#channel;
		 * if none, it is a blank space (' ')
		 * @param ctcp The type of CTCP reply, e.g. "PING" "VERSION" etc.
		 * @param params Any parameters associated with the CTCP reply
		 * @see callChanCTCPReplyOutHooks
		 */
		void modChanCTCPReplyOutHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		
		/** Sent channel CTCP reply hook thread
		 * This function makes sent channel CTCP reply hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when it has sent a CTCP reply to a channel.
		 * @param server The server to which the CTCP reply was sent
		 * @param client The identifier of the client from which the CTCP reply was sent
		 * @param channel The channel to which the CTCP reply was sent
		 * @param status The status char to which the CTCP reply was sent, e.g. '@' if the CTCP reply was sent to @#channel; if none,
		 * it is a blank space (' ')
		 * @param ctcp The type of CTCP reply, e.g. "PING" "VERSION" etc.
		 * @param params Any parameters associated with the CTCP reply
		 * @see callChanCTCPReplySendHooks
		 */
		void modChanCTCPReplySendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		
		/** Outgoing user CTCP reply hook thread
		 * This function makes outgoing user CTCP reply hook calls "nonblocking" by calling the function that actually makes the hook
		 * calls in a new thread.  It is called by a protocol module when it is about to send a CTCP reply to a user.
		 * @param server The server to which the CTCP reply is to be sent
		 * @param client The identifier of the client from which the CTCP reply is to be sent
		 * @param nick The nick of the user to whom the CTCP reply is to be sent
		 * @param ctcp The type of CTCP reply, e.g. "PING" "VERSION" etc.
		 * @param params Any parameters associated with the CTCP reply
		 * @see callUserCTCPReplyOutHooks
		 */
		void modUserCTCPReplyOutHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		/** Sent user CTCP reply hook thread
		 * This function makes sent user CTCP reply hook calls "nonblocking" by calling the function that actually makes the hook calls
		 * in a new thread.  It is called by a protocol module when it has sent a CTCP reply to a user.
		 * @param server The server to which the CTCP reply was sent
		 * @param client The identifier of the client from which the CTCP reply was sent
		 * @param nick The nick of the user to whom the CTCP reply was sent
		 * @param ctcp The type of CTCP reply, e.g. "PING" "VERSION" etc.
		 * @param params Any parameters associated with the CTCP reply
		 * @see callUserCTCPReplySendHooks
		 */
		void modUserCTCPReplySendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		
		/** Send PRIVMSG
		 * Called as a result of a module wanting to send a PRIVMSG, this function forwards the request to the
		 * appropriate protocol module.
		 * @param server The server to send the PRIVMSG to
		 * @param client The identifier of the client from which to send the PRIVMSG
		 * @param target The target of the PRIVMSG
		 * @param message The message to send
		 */
		void sendPrivMsg(std::string server, std::string client, std::string target, std::string message);
		
		/** Send NOTICE
		 * Called as a result of a module wanting to send a NOTICE, this function forwards the request to the
		 * appropriate protocol module.
		 * @param server The server to send the NOTICE to
		 * @param client The identifier of the client from which to send the NOTICE
		 * @param target The target of the NOTICE
		 * @param message The message to send
		 */
		void sendNotice(std::string server, std::string client, std::string target, std::string message);
		
		/** Send CTCP
		 * Called as a result of a module wanting to send a CTCP request, this function forwards the request to the
		 * appropriate protocol module.
		 * @param server The server to send the CTCP request to
		 * @param client The identifier of the client from which to send the CTCP request
		 * @param target The target of the CTCP request
		 * @param ctcp The type of CTCP request to send, e.g. "PING" "VERSION" etc.
		 * @param params Any data to send with the CTCP request
		 */
		void sendCTCP(std::string server, std::string client, std::string target, std::string ctcp, std::string params);
		
		/** Send CTCP reply
		 * Called as a result of a module wanting to send a CTCP reply, this function forwards the request to the
		 * appropriate protocol module.
		 * @param server The server to send the CTCP reply to
		 * @param client The identifier of the client from which to send the CTCP reply
		 * @param target The target of the CTCP reply
		 * @param ctcp The type of CTCP reply to send, e.g. "PING" "VERSION" etc.
		 * @param params Any data to send with the CTCP reply
		 */
		void sendCTCPReply(std::string server, std::string client, std::string target, std::string ctcp, std::string params);
		
		/** Set mode
		 * Called as a result of a module wanting to set a mode, this function forwards the request to the appropriate
		 * protocol module.
		 * @param server The server on which the mode is to be set
		 * @param client The identifier of the client from which to send the MODE
		 * @param setModes List of modes to set; each mode is separated from its parameter (if it has one) with an equals sign
		 * @param delModes List of modes to unset; each mode is separated from its parameter (if required) with an equals sign
		 */
		void setMode(std::string server, std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes);
		
		/** Set SNOmask
		 * Called as a result of a module wanting to set a SNOmask, this function forwards the request to the
		 * appropriate protocol module.
		 * @param server The server on which to change SNOmasks
		 * @param client The identifier of the client for which SNOmasks should be changed
		 * @param snomask The SNOmask to modify
		 * @param add Whether to set or unset it; true to set, false to unset
		 */
		void setSNOMask(std::string server, std::string client, std::string snomask, bool add);
		
		/** Set channel topic
		 * Called as a result of a module wanting to set a channel topic, this function forwards the request to
		 * the appropriate protocol module.
		 * @param server The server on which to change channel topics
		 * @param client The identifier of the client to change the topic
		 * @param channel The channel whose topic should be changed
		 * @param topic The new channel topic
		 */
		void setChanTopic(std::string server, std::string client, std::string channel, std::string topic);
		
		/** Join channel
		 * Called as a result of a module wanting to join a channel, this function forwards the request to the
		 * appropriate protocol module.
		 * @param server The server on which to join a channel
		 * @param client The identifier of the client to join the channel
		 * @param channel The channel to join
		 * @param key The channel key
		 */
		void joinChannel(std::string server, std::string client, std::string channel, std::string key);
		
		/** Part channel
		 * Called as a result of a module wanting to part a channel, this function forwards the request to the
		 * appropriate protocol module.
		 * @param server The server on which to part a channel
		 * @param client The identifier of the client to part the channel
		 * @param channel The channel to part
		 * @param reason The part reason
		 */
		void partChannel(std::string server, std::string client, std::string channel, std::string reason);
		
		/** Add client
		 * Called when a module wants the bot to introduce a new client to a server, this function forwards the
		 * request to the appropriate protocol module.
		 * @param server The server on which to add a client
		 * @param nick The nick of the new client
		 * @param ident The ident of the new client
		 * @param host The host of the new client
		 * @param gecos The GECOS of the new client
		 * @return A string identifier that can be used to refer to that client on that server
		 */
		std::string addClient(std::string server, std::string nick, std::string ident, std::string host, std::string gecos);
		
		/** Remove client
		 * Called when a module wants to remove a client from a server, this function forwards the request to
		 * the appropriate protocol module.
		 * @param server The server on which to remove a client
		 * @param client The identifier of the client to remove
		 */
		void removeClient(std::string server, std::string client);
		
		/** Change nick
		 * Called when a module wants to change a client's nick, this function forwards the request to the
		 * appropriate protocol module.
		 * @param server The server on which to change a nick
		 * @param client The identifier of the client whose nick we wish to change
		 * @param newNick The new nick for the client
		 */
		void changeNick(std::string server, std::string client, std::string newNick);
		
		/** Oper
		 * Called when a module wants a client to oper up on a server, this function forwards the request to
		 * the appropriate protocol module.
		 * @param server The server on which to oper
		 * @param client The identifier of the client to oper
		 * @param username The username or oper type (depending on the module in question) to use in opering up
		 * @param password The oper password when opering up
		 */
		void oper(std::string server, std::string client, std::string username, std::string password);
		
		/** Send server notice
		 * Called when a module wants to send a SNOtice, this function forwards the request to the appropriate
		 * protocol module.
		 * @param server The server to which to send the SNOtice
		 * @param snomask The SNOmask to which to send the notice
		 * @param message The notice message
		 */
		void sendSNotice(std::string server, std::string snomask, std::string message);
		
		/** Set metadata
		 * Called when a module wants to set metadata on a user or channel, this function forwards the request
		 * to the appropriate protocol module.
		 * @param server The server on which to set metadata
		 * @param target The user or channel on which to set metadata
		 * @param key The metadata key whose value we are changing
		 * @param value The new value for the metadata
		 */
		void setMetadata(std::string server, std::string target, std::string key, std::string value);
		
		/** Set x:line
		 * Called when a module wants to set an x:line, this function forwards the request to the appropriate
		 * protocol module.
		 * @param server The server on which to set an x:line
		 * @param client The identifier of the client setting the x:line
		 * @param linetype The type of x:line to set
		 * @param mask The mask on which to set the x:line
		 * @param duration The duration in seconds of the x:line
		 * @param reason The reason for the x:line
		 */
		void setXLine(std::string server, std::string client, std::string linetype, std::string mask, time_t duration, std::string reason);
		
		/** Remove x:line
		 * Called when a module wants to remove an x:line, this function forwards the request to the appropriate
		 * protocol module.
		 * @param server The server on which to remove an x:line
		 * @param client The identifier of the client removing the x:line
		 * @param linetype The type of x:line being removed
		 * @param mask The mask on which to remove the x:line
		 */
		void delXLine(std::string server, std::string client, std::string linetype, std::string mask);
		
		/** Send other data
		 * Called when a module wants to send data to the server that can't be handled by any other functions, this
		 * function forwards the request to the appropriate protocol module.
		 * @param server The server to which to send the data
		 * @param client The identifier of the client to send the data
		 * @param line The data to send
		 */
		void sendOtherData(std::string server, std::string client, std::string line);
	private:
		/** Directory variables
		 * workingDir: The directory of the executable relative to the directory from which robobo was started
		 * configDir: The directory from workingDir to the config
		 * configName: The name of the config file
		 */
		const std::string workingDir, configDir, configName;
		/// The debug level at which the bot is running
		const unsigned short debugLevel;
		/// Whether to dump logs (specified by the --log startup option)
		const bool logDump;
		/// Whether the bot is in startup mode; true while starting up, set to false once everything is running
		bool startup;
		/** Config variables
		 * serverConfig: holds server configuration
		 * moduleConfig: holds module configuration
		 */
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> serverConfig, moduleConfig;
		/** Startup variables
		 * startupServers: holds a list of servers to connect on startup
		 * startupModules: holds a list of modules to load on startup
		 */
		std::list<std::string> startupServers, startupModules;
		/// Servers currently loaded
		std::map<std::string, Protocol*> servers;
		/// The modules currently loaded, separated by priority
		std::map<std::string, Module*> highModules, mediumHighModules, normalModules, mediumLowModules, lowModules;
		/// The module file handles returned by dlopen() so that modules can be properly unloaded
		std::unordered_map<std::string, void*> moduleFiles;
		/// Stored module priority by module name
		std::map<std::string, Priority> modulePriority;
		/** Service variables
		 * moduleServices: holds a list of modules providing each given service
		 * moduleSupports: holds a list of modules using each given service
		 */
		std::map<std::string, std::list<std::string>> moduleServices, moduleSupports;
		/// Holds a description for each module
		std::map<std::string, std::string> moduleDescriptions;
		/// The socket file handles returned by dlopen() for each socket type
		std::unordered_map<std::string, void*> socketFiles;
		/// The mutex ensuring that module hooks are called one at a time
		std::mutex modHookMutex;
		
		/** Channel message hook handler
		 * This function calls the hooks in modules for channel messages, properly handling when a module indicates to ignore a message.
		 * @param server The server from which a channel message was received
		 * @param client The identifier of the client that received the channel message
		 * @param channel The channel through which the message was received
		 * @param status The status char to which the message was sent, e.g. '@' if the message was sent to @#channel; ' ' (space) if
		 * none (sent to #channel)
		 * @param nick The nick of the user who sent the message
		 * @param message The message
		 * @see modChanMsgHook
		 */
		void callChanMsgHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		
		/** User message hook handler
		 * This function calls the hooks in modules for user messages, properly handling when a module indicates to ignore a message.
		 * @param server The server from which a user message was received
		 * @param client The identifier of the client that received the user message
		 * @param nick The nick of the user who sent the message
		 * @param message The message
		 * @see modUserMsgHook
		 */
		void callUserMsgHooks(std::string server, std::string client, std::string nick, std::string message);
		
		/** Channel notice hook handler
		 * This function calls the hooks in modules for channel notices, properly handling when a module indicates to ignore a message.
		 * @param server The server from which a channel notice was received
		 * @param client The identifier of the client that received the notice
		 * @param channel The channel through which the notice was received
		 * @param status The status char to which the notice was sent, e.g. '@' if the message was sent to @#channel; ' ' (space) if none
		 * (sent to #channel)
		 * @param nick The nick of the user who sent the notice
		 * @param message The message
		 * @see modChanNoticeHook
		 */
		void callChanNoticeHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		
		/** User notice hook handler
		 * This function calls the hooks in modules for user notices, properly handling when a module indicates to ignore a message.
		 * @param server The server from which a user notice was received
		 * @param client The identifier of the client that received the notice
		 * @param nick The nick of the user who sent the notice
		 * @param message The message
		 * @see modUserNoticeHook
		 */
		void callUserNoticeHooks(std::string server, std::string client, std::string nick, std::string message);
		
		/** Channel CTCP hook handler
		 * This function calls the hooks in modules for channel CTCP requests, properly handling when a module indicates to ignore a message.
		 * @param server The server from which a channel CTCP request was received
		 * @param client The identifier of the client that received the CTCP request
		 * @param channel The channel through which the CTCP request was received
		 * @param status The status char to which the CTCP request was sent, e.g. '@' if the message was sent to @#channel; ' ' (space)
		 * if none (sent to #channel)
		 * @param nick The nick of the user who sent the CTCP request
		 * @param ctcp The type of CTCP request ("PING" "VERSION" etc.)
		 * @param data Any parameters associated with the CTCP request
		 * @see modChanCTCPHook
		 */
		void callChanCTCPHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		
		/** User CTCP hook handler
		 * This function calls the hooks in modules for user CTCP requests, properly handling when a module indicates to ignore a message.
		 * @param server The server from which a user CTCP request was received
		 * @param client The identifier of the client that received the CTCP request
		 * @param nick The nick of the user who sent the CTCP request
		 * @param ctcp The type of CTCP request ("PING" "VERSION" etc.)
		 * @param data Any parameters associated with the CTCP request
		 * @see modUserCTCPHook
		 */
		void callUserCTCPHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		
		/** Channel CTCP reply hook handler
		 * This function calls the hooks in modules for channel CTCP replies, properly handling when a module indicates to ignore a message.
		 * @param server The server from which a channel CTCP reply was received
		 * @param client The identifier of the client that received the CTCP reply
		 * @param channel The channel through which the CTCP reply was received
		 * @param status The status char to which the CTCP reply was sent, e.g. '@' if the message was sent to @#channel; ' ' (space) if none
		 * (sent to #channel)
		 * @param nick The nick of the user who sent the CTCP reply
		 * @param ctcp The type of CTCP reply ("PING" "VERSION" etc.)
		 * @param data Any data associated with the CTCP reply
		 * @see modChanCTCPReplyHook
		 */
		void callChanCTCPReplyHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		
		/** User CTCP reply hook handler
		 * This function calls the hooks in modules for user CTCP replies, properly handling when a module indicates to ignore a message.
		 * @param server The server from which a user CTCP reply was received
		 * @param client The identifier of the client that received the CTCP reply
		 * @param nick The nick of the user who sent the CTCP reply
		 * @param ctcp The type of CTCP reply ("PING" "VERSION" etc.)
		 * @param data Any data associated with the CTCP reply
		 * @see modUserCTCPReplyHook
		 */
		void callUserCTCPReplyHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		
		/** Channel mode hook handler
		 * This function calls the hooks in modules for modes set in channels.
		 * @param server The server on which a channel mode was changed
		 * @param client The identifier of the client that received notice of a channel mode change
		 * @param channel The channel in which the mode was changed
		 * @param setter The nick of the user who changed the channel mode
		 * @param add Whether the mode is being set or unset, true if set
		 * @param mode The long name of the mode being changed
		 * @param param The mode parameter, if given; empty string if no parameter
		 * @see modChanModeHook
		 */
		void callChanModeHooks(std::string server, std::string client, std::string channel, std::string setter, bool add, std::string mode, std::string param);
		
		/** User mode hook handler
		 * This function calls the hooks in modules for modes set on a user.
		 * @param server The server on which a user mode was changed
		 * @param client The identifier of the client that had its user modes changed
		 * @param add Whether the mode is being set or unset, true if set
		 * @param mode The long name of the mode being changed
		 * @see modUserModeHook
		 */
		void callUserModeHooks(std::string server, std::string client, bool add, std::string mode);
		
		/** User SNOmask hook handler
		 * This function calls the hooks in modules for SNOmasks set on a user.
		 * @param server The server on which a SNOmask was changed
		 * @param client The identifier of the client that had its SNOmask changed
		 * @param add Whether the SNOmask is being set or unset, true if set
		 * @param snomask The SNOmask being set
		 * @see modUserSNOMaskHook
		 */
		void callUserSNOMaskHooks(std::string server, std::string client, bool add, std::string snomask);
		
		/** Channel topic hook handler
		 * This function calls the hooks in modules for channel topic changes.
		 * @param server The server on which a topic was changed
		 * @param client The identifier of the client that saw the topic change
		 * @param channel The channel in which the topic was changed
		 * @param setter The nick of the user who changed the topic
		 * @param topic The new channel topic
		 * @see modChanTopicHook
		 */
		void callChanTopicHooks(std::string server, std::string client, std::string channel, std::string setter, std::string topic);
		
		/** Channel join hook handler
		 * This function calls the hooks in modules for channel joins.
		 * @param server The server on which a channel was joined.
		 * @param client The identifier of the client that saw the channel join
		 * @param channel The channel that was joined
		 * @param nick The nick of the user who joined
		 * @see modChanJoinHook
		 */
		void callChanJoinHooks(std::string server, std::string client, std::string channel, std::string nick);
		
		/** Channel part hook handler
		 * This function calls the hooks in modules for channel parts.
		 * @param server The server on which a channel was parted.
		 * @param client The identifier of the client that saw the channel part
		 * @param channel The channel that was parted
		 * @param nick The nick of the user who parted
		 * @param reason The part reason given
		 * @see modChanPartHook
		 */
		void callChanPartHooks(std::string server, std::string client, std::string channel, std::string nick, std::string reason);
		
		/** User connect hook handler
		 * This function calls the hooks in modules for user connections.
		 * @param server The server on the network to which the user connected
		 * @param nick The nick of the user who connected
		 * @see modUserConnectHook
		 */
		void callUserConnectHooks(std::string server, std::string nick);
		
		/** User quit hook handler
		 * This function calls the hooks in modules for user quits.
		 * @param server The server on which the quit was seen
		 * @param client The identifier of the client that saw the quit
		 * @param nick The nick of the user who quit
		 * @param reason The quit reason given
		 * @see modUserQuitHook
		 */
		void callUserQuitHooks(std::string server, std::string client, std::string nick, std::string reason);
		
		/** User nick change hook handler
		 * This function calls the hooks in modules for user nick changes.
		 * @param server The server on which the nick was changed
		 * @param client The client that saw the nick change
		 * @param oldNick The old nick of the user
		 * @param newNick The new nick of the user
		 * @see modUserNickHook
		 */
		void callUserNickHooks(std::string server, std::string client, std::string oldNick, std::string newNick);
		
		/** Numeric hook handler
		 * This function calls the hooks in modules for numerics.
		 * @param server The server from which we received the numeric
		 * @param client The identifier of the client that received the numeric
		 * @param numeric The numeric
		 * @param data Any further data sent with the numeric, split into tokens
		 * @see modNumericHook
		 */
		void callNumericHooks(std::string server, std::string client, std::string numeric, std::vector<std::string> data);
		
		/** Oper hook handler
		 * This function calls the hooks in modules for users opering up.
		 * @param server The server on which the user opered.
		 * @param nick The nick of the user who is opered.
		 * @param operType The opertype of the user.
		 * @see modOperHook
		 */
		void callOperHooks(std::string server, std::string nick, std::string operType);
		
		/** SNOtice hook handler
		 * This function calls the hooks in modules for server notices.
		 * @param server The server on which the SNOtice was sent
		 * @param snotype The SNOmask to which the notice is sent
		 * @param message The message being sent
		 * @see modSNoticeHook
		 */
		void callSNoticeHooks(std::string server, std::string snotype, std::string message);
		
		/** Metadata hook handler
		 * This function calls the hooks in modules for changes in metadata.
		 * @param server The server on which metadata was changed
		 * @param target The user or channel whose metadata was changed
		 * @param dataKey The metadata key whose value was changed
		 * @param dataValue The new value of the metadata
		 * @see modMetadataHook
		 */
		void callMetadataHooks(std::string server, std::string target, std::string dataKey, std::string dataValue);
		
		/** x:line add hook handler
		 * This function calls the hooks in modules for new x:lines.
		 * @param server The server on which the x:line was added
		 * @param lineType The type of x:line added
		 * @param mask The mask on which the x:line was added
		 * @param setter The user who set the x:line
		 * @param expiry The time at which the x:line expires
		 * @param reason The reason given for the x:line
		 * @see modXLineAddHook
		 */
		void callXLineAddHooks(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason);
		
		/** x:line remove hook handler
		 * This function calls the hooks in modules for removed x:lines.
		 * @param server The server on which the x:line was removed
		 * @param lineType The type of x:line that was removed
		 * @param mask The mask on which the removed x:line was
		 * @see modXLineRemoveHook
		 */
		void callXLineRemoveHooks(std::string server, std::string lineType, std::string mask);
		
		/** Server connect hook handler
		 * This function calls the hooks in modules for new servers connected to the network.
		 * @param server The server on the network to which we are connected
		 * @param newServerName The name of the server connecting to the network
		 * @see modServerConnectHook
		 */
		void callServerConnectHooks(std::string server, std::string newServerName);
		
		/** Server quit hook handler
		 * This function calls the hooks in modules for servers quitting from the network.
		 * @param server The server on the network to which we are connected
		 * @param quitServerName The name of the server quitting from the network
		 * @param reason The reason given for the quit
		 * @see modServerQuitHook
		 */
		void callServerQuitHooks(std::string server, std::string quitServerName, std::string reason);
		
		/** Other data hook handler
		 * This function calls the hooks in modules for data that cannot be handled by other hooks.
		 * @param server The server from which the data was received
		 * @param client The identifier of the client that received the data
		 * @param lineTokens The line received split into IRC tokens
		 * @see modOtherDataHook
		 */
		void callOtherDataHooks(std::string server, std::string client, std::vector<std::string> lineTokens);
		
		/** Outgoing channel message hook handler
		 * This function calls the hooks in modules for outgoing channel messages, handling requested changes to the message.
		 * @param server The server to which the message is to be sent
		 * @param client The identifier of the client from which the message is to be sent
		 * @param channel The channel to which the message is to be sent
		 * @param status The status char to which the message is to be sent, e.g. '@' if the message is to be sent to @#channel;
		 * if none, it is a space (' ')
		 * @param message The message to be sent
		 * @see modChanMsgOutHook
		 */
		void callChanMsgOutHooks(std::string server, std::string client, std::string channel, char status, std::string message);
		
		/** Sent channel message hook handler
		 * This function calls the hooks in modules for sent channel messages.
		 * @param server The server to which the message was sent
		 * @param client The identifier of the client from which the message was sent
		 * @param channel The channel to which the message was sent
		 * @param status The status char to which the message was sent, e.g. '@' if the message was sent to @#channel; if none,
		 * it is a space (' ')
		 * @param message The message that was sent
		 * @see modChanMsgSendHook
		 */
		void callChanMsgSendHooks(std::string server, std::string client, std::string channel, char status, std::string message);
		
		/** Outgoing user message hook handler
		 * This function calls the hooks in modules for outgoing user messages, handling requested changes to the message.
		 * @param server The server to which the message is to be sent
		 * @param client The identifier of the client from which the message is to be sent
		 * @param nick The nick of the user to whom the message is to be sent
		 * @param message The message to be sent
		 * @see modUserMsgOutHook
		 */
		void callUserMsgOutHooks(std::string server, std::string client, std::string nick, std::string message);
		
		/** Sent user message hook handler
		 * This function calls the hooks in modules for sent user messages.
		 * @param server The server to which the message was sent
		 * @param client The identifier of the client from which the message was sent
		 * @param nick The nick of the user to whom the message was sent
		 * @param message The message that was sent
		 * @see modUserMsgSendHook
		 */
		void callUserMsgSendHooks(std::string server, std::string client, std::string nick, std::string message);
		
		/** Outgoing channel notice hook handler
		 * This function calls the hooks in modules for outgoing channel notices, handling requested changes to the message.
		 * @param server The server to which the notice is to be sent
		 * @param client The identifier of the client from which the notice is to be sent
		 * @param channel The channel to which the notice is to be sent
		 * @param status The status char to which the notice is to be sent, e.g. '@' if the notice is to be sent to
		 * @#channel; if none, it is a space (' ')
		 * @param message The message to be sent
		 * @see modChanNoticeOutHook
		 */
		void callChanNoticeOutHooks(std::string server, std::string client, std::string channel, char status, std::string message);
		
		/** Sent channel notice hook handler
		 * This function calls the hooks in modules for sent channel notices.
		 * @param server The server to which the notice was sent
		 * @param client The identifier of the client from which the notice was sent
		 * @param channel The channel to which the notice was sent
		 * @param status The status char to which the notice was sent, e.g. '@' if the notice was sent to @#channel; if
		 * none, it is a space (' ')
		 * @param message The message that was sent
		 * @see modChanNoticeSendHook
		 */
		void callChanNoticeSendHooks(std::string server, std::string client, std::string channel, char status, std::string message);
		
		/** Outgoing user notice hook handler
		 * This function calls the hooks in modules for outgoing user notices, handling requested changes to the message.
		 * @param server The server to which the notice is to be sent
		 * @param client The identifier of the client from which the notice is to be sent
		 * @param nick The nick of the user to whom the message is to be sent
		 * @param message The message to be sent
		 * @see modUserNoticeOutHook
		 */
		void callUserNoticeOutHooks(std::string server, std::string client, std::string nick, std::string message);
		
		/** Sent user notice hook handler
		 * This function calls the hooks in modules for sent user notices.
		 * @param server The server to which the notice was sent
		 * @param client The identifier of the client from which the notice was sent
		 * @param nick The nick of the user to whom the message was sent
		 * @param message The message that was sent
		 * @see modUserNoticeSendHook
		 */
		void callUserNoticeSendHooks(std::string server, std::string client, std::string nick, std::string message);
		
		/** Outgoing channel CTCP hook handler
		 * This function calls the hooks in modules for outgoing channel CTCP requests, handling requested changes to the CTCP
		 * request and associated data.
		 * @param server The server to which the CTCP request is to be sent
		 * @param client The identifier of the client sending the CTCP request
		 * @param channel The channel to which the CTCP request is to be sent
		 * @param status The status char to which the CTCP request is to be sent, e.g. '@' if the CTCP request is to be sent
		 * to @#channel; if none, it is a space (' ')
		 * @param ctcp The type of CTCP request to be sent ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP request
		 * @see modChanCTCPOutHook
		 */
		void callChanCTCPOutHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		
		/** Sent channel CTCP hook handler
		 * This function calls the hooks in modules for sent channel CTCP requests.
		 * @param server The server to which the CTCP request was sent
		 * @param client The identifier of the client from which the CTCP request was sent
		 * @param channel The channel to which the CTCP request was sent
		 * @param status The status char to which the CTCP request was sent, e.g. '@' if the CTCP request was sent to @#channel;
		 * if none, it is a space (' ')
		 * @param ctcp The type of CTCP request sent ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP request
		 * @see modChanCTCPSendHook
		 */
		void callChanCTCPSendHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		
		/** Outgoing user CTCP hook handler
		 * This function calls the hooks in modules for outgoing user CTCP requests, handling requested changes to the CTCP
		 * request and associated data.
		 * @param server The server to which the CTCP request is to be sent
		 * @param client The identifier of the client from which the CTCP request is to be sent
		 * @param nick The nick of the user to whom the CTCP request is to be sent
		 * @param ctcp The type of CTCP request to be sent ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP request
		 * @see modUserCTCPOutHook
		 */
		void callUserCTCPOutHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		/** Sent user CTCP hook handler
		 * This function calls the hooks in modules for sent user CTCP requests.
		 * @param server The server to which the CTCP request was sent
		 * @param client The identifier of the client from which the CTCP request was sent
		 * @param nick The nick of the user to whoom the CTCP request was sent
		 * @param ctcp The type of CTCP request sent ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP request
		 * @see modUserCTCPSendHook
		 */
		void callUserCTCPSendHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		/** Outgoing channel CTCP reply hook handler
		 * This function calls the hooks in modules for outgoing channel CTCP replies, handling requested changes to the CTCP reply
		 * and associated data.
		 * @param server The server to which the CTCP reply is to be sent
		 * @param client The identifier of the client from which the CTCP reply is to be sent
		 * @param channel The channel to which the CTCP reply is to be sent
		 * @param status The status char to which the CTCP reply is to be sent, e.g. '@' if the CTCP request is to be sent to
		 * @#channel; if none, it is a space (' ')
		 * @param ctcp The type of CTCP reply to be sent ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP reply
		 * @see modChanCTCPReplyOutHook
		 */
		void callChanCTCPReplyOutHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		
		/** Sent channel CTCP reply hook handler
		 * This function calls the hooks in modules for sent channel CTCP replies.
		 * @param server The server to which the CTCP reply was sent
		 * @param client The identifier of the client from which the CTCP reply was sent
		 * @param channel The channel to which the CTCP reply was sent
		 * @param status The status char to which the CTCP reply was sent, e.g. '@' if the CTCP reply was sent to @#channel; if
		 * none, it is a space (' ')
		 * @param ctcp The type of CTCP reply sent ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP reply
		 * @see modChanCTCPReplySendHook
		 */
		void callChanCTCPReplySendHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		
		/** Outgoing user CTCP reply hook handler
		 * This function calls the hooks in modules for outgoing user CTCP replies, handling requested changes to the CTCP reply and
		 * associated data.
		 * @param server The server to which the CTCP reply is to be sent
		 * @param client The identifer of the client from which the CTCP reply is to be sent
		 * @param nick The nick of the user to whom the CTCP reply is to be sent
		 * @param ctcp The type of CTCP reply to be sent ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP reply
		 * @see modUserCTCPReplyOutHook
		 */
		void callUserCTCPReplyOutHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		/** Sent user CTCP reply hook handler
		 * This function calls the hooks in modules for sent user CTCP replies.
		 * @param server The server to which the CTCP reply was sent
		 * @param client The identifier of the client from which the CTCP reply was sent
		 * @param nick The nick of the user to whom the CTCP reply was sent
		 * @param ctcp The type of CTCP reply that was sent ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP reply
		 * @see modUserCTCPReplySendHook
		 */
		void callUserCTCPReplySendHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
};