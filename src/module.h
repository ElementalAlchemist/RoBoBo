#pragma once
#include "main.h"

#include "base.h"

/** Module priority
 * A flag used by modules to help determine the order in which module hooks are called.
 */
enum Priority { PRI_HIGH, PRI_MEDIUM_HIGH, PRI_NORMAL, PRI_MEDIUM_LOW, PRI_LOW };

/// Define the spawn function for module files
#define MODULE_SPAWN(modName) extern "C" Module* spawn(std::string moduleName, std::map<std::string, std::string> config, std::string workingDir, unsigned short debugLevel, Base* botptr) {\
		return new modName (moduleName, config, workingDir, debugLevel, botptr);\
	}

/** Message action
 * A flag returned by modules with received messages such that messages can be ignored or not.
 */
typedef bool MsgAction;
const bool MSG_CONTINUE = true;
const bool MSG_IGNORE = false;

class Module {
	public:
		/** Module constructor
		 * Fills the Module class with the basic things it needs to function
		 * @param modName The name of the module
		 * @param conf The module configuration
		 * @param workDir The working directory of the bot relative to the directory from which it was started
		 * @param debug The current debug flag of the bot
		 * @param botptr A pointer to the bot base for communication with the rest of the bot
		 */
		Module(std::string modName, std::map<std::string, std::string> conf, std::string workDir, unsigned short debug, Base* botptr);
		
		/** Module destructor
		 * As this is the base module class from which modules will be derived, the destructor is virtual.
		 */
		virtual ~Module();
		
		/** The module's priority
		 * @see enum Priority
		 */
		const Priority priority;
		
		/// The module API version with which the module is compatible
		virtual unsigned int apiVersion() = 0;
		
		/** Load completion hook
		 * This function is called when the module has finished being loaded so that modules can do whatever startup
		 * things they need to.
		 * @return true if the module loaded successfully; false if it did not.
		 */
		virtual bool onLoadComplete();
		
		/** Unload hook
		 * This function is called when the module is about to be unloaded.
		 */
		virtual void onUnload();
		
		/** Rehash hook
		 * This function is called when the bot has just rehashed.  This allows the bot to react to changes in the
		 * configuration if it has a special setup to do so.
		 */
		virtual void onRehash();
		
		/** Module load hook
		 * This function is called when a different module loads, allowing modules to react to the loading of another
		 * module.
		 * @param modName The name of the module being loaded.
		 */
		virtual void onModuleLoad(std::string modName);
		
		/** Module unload hook
		 * This function is called when a different module unloads, allowing modules to react to the unloading of
		 * another module, e.g. to kill itself if it depends on the module being unloaded.
		 * @param modName The name of the module being unloaded.
		 */
		virtual void onModuleUnload(std::string modName);
		
		/** Keep bot alive
		 * If, for some reason, the module needs to keep the bot alive even after all servers have disconnected, it
		 * can do so by returning true here.  This should be a rare case, but it could have some use.
		 * @return Whether to force the bot staying alive
		 */
		virtual bool forceKeepAlive();
		
		/** Rehash function
		 * This function replaces the existing configuration of the module with a new one loaded from the configuration.
		 * The onRehash hook is called immediately after.
		 * @param conf The new configuration read from the config file.
		 * @see onRehash
		 */
		void rehash(std::map<std::string, std::string> conf);
		
		/** End debug notifier
		 * If, at any time, the bot leaves debug mode, this function will be called in all modules to set the debug
		 * level to 0.
		 * @see debugLevel
		 */
		void endDebug();
		
		
		/** Channel message hook
		 * This function is called when a channel message is received by the bot, allowing modules to react.
		 * @param server The server from which the message was received
		 * @param client The identifier of the client that received the message
		 * @param channel The channel through which it was received
		 * @param status The status char to which the message was sent, e.g. '@' if it was sent to @#channel; if none,
		 * it is a space (' ')
		 * @param nick The nick of the user who sent the message
		 * @param message The message sent
		 * @return MSG_CONTINUE to continue processing the message in other modules, MSG_IGNORE to stop
		 */
		virtual MsgAction onChanMsg(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		
		/** User message hook
		 * This function is called when a user message is received by the bot, allowing modules to react.
		 * @param server The server from which the message was received
		 * @param client The identifier of the client that received the message
		 * @param nick The nick of the user who sent the message
		 * @param message The message sent
		 * @return MSG_CONTINUE to continue processing the message in other modules, MSG_IGNORE to stop
		 */
		virtual MsgAction onUserMsg(std::string server, std::string client, std::string nick, std::string message);
		
		/** Channel notice hook
		 * This function is called when a channel notice is received by the bot, allowing modules to react.
		 * @param server The server from which the notice was received
		 * @param client The identifier of the client that received the notice
		 * @param channel The channel through which it was received
		 * @param status The status char to which the message was sent, e.g. '@' if it was sent to @#channel; if none, it
		 * is a space (' ')
		 * @param nick The nick of the user who sent the notice
		 * @param message The message sent
		 * @return MSG_CONTINUE to continue processing the notice in other modules, MSG_IGNORE to stop
		 */
		virtual MsgAction onChanNotice(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		
		/** User notice hook
		 * This function is called when a user notice is received by the bot, allowing modules to react.
		 * @param server The server from which the notice was received
		 * @param client The identifier of the client that received the notice
		 * @param nick The nick of the user who sent the notice
		 * @param message The message sent
		 * @return MSG_CONTINUE to continue processing the notice in other modules, MSG_IGNORE to stop
		 */
		virtual MsgAction onUserNotice(std::string server, std::string client, std::string nick, std::string message);
		
		/** Channel CTCP hook
		 * This function is called when a channel CTCP request is received by the bot, allowing modules to react.
		 * @param server The server from which the CTCP request was received
		 * @param client The identifier of the client that received the CTCP request
		 * @param channel The channel through which the CTCP request was received
		 * @param status The status char to which the CTCP request was sent, e.g. '@' if it was sent to @#channel; if
		 * none, it is a space (' ')
		 * @param nick The nick of the user who sent the CTCP request
		 * @param ctcp The type of CTCP request ("PING" "VERSION" etc.)
		 * @param data Any data associated with the CTCP request
		 * @return MSG_CONTINUE to continue processing the CTCP request in other modules, MSG_IGNORE to stop
		 */
		virtual MsgAction onChanCTCP(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		
		/** User CTCP hook
		 * This function is called when a user CTCP request is received by the bot, allowing modules to react.
		 * @param server The server from which the CTCP request was received
		 * @param client The identifier of the client that received the CTCP request
		 * @param nick The nick of the user who sent the CTCP request
		 * @param ctcp The type of CTCP request ("PING" "VERSION" etc.)
		 * @param data Any data associated with the CTCP request
		 * @return MSG_CONTINUE to continue processing the CTCP request in other modules, MSG_IGNORE to stop
		 */
		virtual MsgAction onUserCTCP(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		
		/** Channel CTCP reply hook
		 * This function is called when a channel CTCP reply is received by the bot, allowing modules to react.
		 * @param server The server from which the CTCP reply was received
		 * @param client The identifier of the client that received the CTCP reply
		 * @param channel The channel through which the CTCP reply was received
		 * @param status The status char to which the CTCP reply was sent, e.g. '@' if it was sent to @#channel; if none,
		 * it is a space (' ')
		 * @param nick The nick of the user who sent the CTCP reply
		 * @param ctcp The type of CTCP reply ("PING" "VERSION" etc.)
		 * @param data Any data associated with the CTCP reply
		 * @return MSG_CONTINUE to continue processing the CTCP reply in other modules, MSG_IGNORE to stop
		 */
		virtual MsgAction onChanCTCPReply(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		
		/** User CTCP reply hook
		 * This function is called when a user CTCP reply is received by the bot, allowing modules to react.
		 * @param server The server from which the CTCP reply was received
		 * @param client The identifier of the client that received the CTCP reply
		 * @param nick The nick of the user who sent the CTCP reply
		 * @param ctcp The type of CTCP reply ("PING" "VERSION" etc.)
		 * @param data Any data associated with the CTCP reply
		 * @return MSG_CONTINUE to continue processing the CTCP reply in other modules, MSG_IGNORE to stop
		 */
		virtual MsgAction onUserCTCPReply(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		
		/** Channel mode hook
		 * This function is called when a channel mode change is received by the bot, allowing modules to react.
		 * @param server The server on which the channel mode change was issued
		 * @param channel The channel on which the modes were changed
		 * @param setter The nick of the user who changed the modes
		 * @param add Whether the mode was set (true) or unset (false)
		 * @param mode The long name of the mode set or unset
		 * @param param The mode parameter, if given
		 */
		virtual void onChanMode(std::string server, std::string channel, std::string setter, bool add, std::string mode, std::string param);
		
		/** User mode hook
		 * This function is called when a user mode change is received by the bot, allowing modules to react.
		 * @param server The server on which the user mode change was issued
		 * @param nick The nick of the user whose mode was changed
		 * @param add Whether the mode was set (true) or unset (false)
		 * @param mode The long name of the mode set or unset
		 */
		virtual void onUserMode(std::string server, std::string nick, bool add, std::string mode);
		
		/** User SNOmask hook
		 * This function is called when a user SNOmask change is received by the bot, allowing modules to react.
		 * @param server The server on which the SNOmask was changed
		 * @param nick The nick of the user whose SNOmask was changed
		 * @param add Whether the SNOmask type was added (true) or removed (false)
		 * @param snomask The snomask char changed
		 */
		virtual void onUserSNOMask(std::string server, std::string nick, bool add, char snomask);
		
		/** Channel topic hook
		 * This function is called when a channel topic change is received by the bot, allowing modules to react.
		 * @param server The server on which the topic was changed
		 * @param channel The channel whose topic was changed
		 * @param setter The nick of the user who changed the topic
		 * @param topic The new channel topic
		 */
		virtual void onChanTopic(std::string server, std::string channel, std::string setter, std::string topic);
		
		/** Channel join hook
		 * This function is called when a channel join is received by the bot, allowing modules to react.
		 * @param server The server on which a channel was joined
		 * @param channel The channel that was joined
		 * @param nick The nick of the user who joined
		 */
		virtual void onChanJoin(std::string server, std::string channel, std::string nick);
		
		/** Channel part hook
		 * This function is called when a channel part is received by the bot, allowing modules to react.
		 * @param server The server on which a channel was parted
		 * @param channel The channel that was parted
		 * @param nick The nick of the user who parted
		 */
		virtual void onChanPart(std::string server, std::string channel, std::string nick, std::string reason);
		
		/** Channel kick hook
		 * This function is called when a channel kick is received by the bot, allowing modules to react.
		 * @param server The server on which a user was kicked
		 * @param channel The channel from which the user was kicked
		 * @param kicker The nick of the user who kicked
		 * @param kickee The nick of the user who was kicked
		 * @param reason The kick reason
		 */
		virtual void onChanKick(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason);
		
		/** User connect hook
		 * This function is called when a user connect notice is received by the bot, allowing modules to react.
		 * @param server The server on the network to which a user connected
		 * @param nick The nick of the user who connected
		 */
		virtual void onUserConnect(std::string server, std::string nick);
		
		/** User quit hook
		 * This function is called when a user quit notice is received by the bot, allowing modules to react.
		 * @param server The server on the network from which the user quit
		 * @param nick The nick of the user who quit
		 * @param reason The quit reason
		 */
		virtual void onUserQuit(std::string server, std::string nick, std::string reason);
		
		/** User nick hook
		 * This function is called when a nick change is received by the bot, allowing modules to react.
		 * @param server The server on which the nick was changed
		 * @param oldNick The old nick of the user
		 * @param newNick The new nick of the user
		 */
		virtual void onUserNick(std::string server, std::string oldNick, std::string newNick);
		
		/** Numeric hook
		 * This function is called when a numeric is received by the bot, allowing modules to react.
		 * @param server The server from which the numeric was received
		 * @param client The identifier of the client that received the numeric
		 * @param numeric The numeric received
		 * @param data The parameters given with the numeric split by IRC tokens
		 */
		virtual void onNumeric(std::string server, std::string client, std::string numeric, std::vector<std::string> data);
		
		/** Oper hook
		 * This function is called when notice of a user being opered is received by the bot, allowing modules to react.
		 * @param server The server on which a user was opered
		 * @param nick The nick of the user who opered
		 * @param operType The type of oper the user now is
		 */
		virtual void onOper(std::string server, std::string nick, std::string operType);
		
		/** Server notice hook
		 * This function is called when a server notice is received by the bot, allowing modules to react.
		 * @param server The server from which the SNOtice was received
		 * @param snotype The snomask under which the notice was sent
		 * @param message The message of the notice
		 */
		virtual void onSNotice(std::string server, char snotype, std::string message);
		
		/** Metadata hook
		 * This function is called when user or channel metadata is received by the bot, allowing modules to react.
		 * @param server The server from which the metadata was received
		 * @param target The user or channel to which the metadata is being assigned
		 * @param dataKey The type of metadata being set
		 * @param dataValue The new value of the metadata
		 */
		virtual void onMetadata(std::string server, std::string target, std::string dataKey, std::string dataValue);
		
		/** x:line add hook
		 * This function is called when the addition of an x:line is received by the bot, allowing modules to react.
		 * @param server The server on which the x:line was added
		 * @param lineType The type of x:line added
		 * @param mask The mask on which the x:line was added
		 * @param setter The setter of the x:line
		 * @param expiry The time at which the x:line expires
		 * @param reason The reason for the x:line
		 */
		virtual void onXLineAdd(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason);
		
		/** x:line remove hook
		 * This function is called when the removal of an x:line is received by the bot, or when an x:line expires, allowing modules to react.
		 * @param server The server on which the x:line expired or was removed
		 * @param lineType The type of x:line removed or expired
		 * @param mask The mask on which the x:line was
		 */
		virtual void onXLineRemove(std::string server, std::string lineType, std::string mask);
		
		/** Server connect hook
		 * This function is called when notice of a new server is received by the bot, allowing modules to react.
		 * @param server The server on the network to which a new server was introduced
		 * @param newServerName The name of the new server on the network
		 */
		virtual void onServerConnect(std::string server, std::string newServerName);
		
		/** Server quit hook
		 * This function is called when notice of a server splitting is received by the bot, allowing modules to react.
		 * @param server The server on the network from which the server split
		 * @param quitServerName The name of the server that split
		 * @param reason The reason for the netsplit
		 */
		virtual void onServerQuit(std::string server, std::string quitServerName, std::string reason);
		
		/** Other data hook
		 * This function is called when data that cannot be handled by another API function is received, allowing modules to react.
		 * @param server The server from which the data was received
		 * @param client The identifier of the client that received the data
		 * @param lineTokens The line the bot received, split up into IRC tokens
		 */
		virtual void onOtherData(std::string server, std::string client, std::vector<std::string> lineTokens);
		
		
		/** Outgoing channel message hook
		 * This function is called when a channel message is about to be sent to the server, allowing modules to modify the message.
		 * @param server The server to which the message will be sent
		 * @param client The identifier of the client from which the message will be sent
		 * @param channel The channel to which the message will be sent
		 * @param status The status char to which the message will be sent, e.g. '@' if the message is to be sent to @#channel; if none,
		 * it is a blank space (' ')
		 * @param message The message to be sent
		 */
		virtual void onChanMsgOut(std::string server, std::string client, std::string channel, char status, std::string &message);
		
		/** Sent channel message hook
		 * This function is called when a channel message has been sent to the server, allowing modules to react.
		 * @param server The server to which the message was sent
		 * @param client The identifier of the client from which the message was sent
		 * @param channel The channel to which the message was sent
		 * @param status The status char to which the message was sent, e.g. '@' if the message was sent to @#channel; if none, it is
		 * a blank space (' ')
		 * @param message The message that was sent
		 */
		virtual void onChanMsgSend(std::string server, std::string client, std::string channel, char status, std::string message);
		
		/** Outgoing user message hook
		 * This function is called when a user message is about to be sent to the server, allowing modules to modify the message.
		 * @param server The server to which the message will be sent
		 * @param client The identifier of the client to send the message
		 * @param nick The nick of the user to whom the message will be sent
		 * @param message The message to be sent
		 */
		virtual void onUserMsgOut(std::string server, std::string client, std::string nick, std::string &message);
		
		/** Sent user message hook
		 * This function is called when a user message has been sent to the server, allowing modules to react.
		 * @param server The server to which the message was sent
		 * @param client The identifier of the client from which the message was sent
		 * @param nick The nick of the user to whom the message was sent
		 * @param message The message that was sent
		 */
		virtual void onUserMsgSend(std::string server, std::string client, std::string nick, std::string message);
		
		/** Outgoing channel notice hook
		 * This function is called when a channel notice is about to be sent to the server, allowing modules to modify the message
		 * @param server The server to which the message was sent
		 * @param client The identifier of the client from which the notice will be sent
		 * @param channel The channel to which the notice will be sent
		 * @param status The status char to which the message will be sent, e.g. '@' if the message will be sent to @#channel; if
		 * none, it is a blank space (' ')
		 * @param message The message to be sent
		 */
		virtual void onChanNoticeOut(std::string server, std::string client, std::string channel, char status, std::string &message);
		
		/** Sent channel notice hook
		 * This function is called when a channel notice has been sent to the server, allowing modules to react.
		 * @param server The server to which the notice was sent
		 * @param client The identifier of the client from which the notice was sent
		 * @param channel The channel to which the notice was sent
		 * @param status The status char to which the notice was sent, e.g. '@' if the notice was sent to @#channel; if none, it is
		 * a blank space (' ')
		 * @param message The message that was sent
		 */
		virtual void onChanNoticeSend(std::string server, std::string client, std::string channel, char status, std::string message);
		
		/** Outgoing user notice hook
		 * This function is called when a user notice is about to be sent to the server, allowing modules to modify the message.
		 * @param server The server to which the notice will be sent
		 * @param client The identifier of the client from which the notice will be sent
		 * @param nick The nick of the user to whom the notice will be sent
		 * @param message The message to be sent
		 */
		virtual void onUserNoticeOut(std::string server, std::string client, std::string nick, std::string &message);
		
		/** Sent user notice hook
		 * This function is called when a user notice has been sent to the server, allowing modules to react.
		 * @param server The server to which the notice was sent
		 * @param client The identifier of the client from which the notice was sent
		 * @param nick The nick of the user to whom the notice was sent
		 * @param message The message that was sent
		 */
		virtual void onUserNoticeSend(std::string server, std::string client, std::string nick, std::string message);
		
		/** Outgoing channel CTCP hook
		 * This function is called when a channel CTCP request is about to be sent to the server, allowing modules to modify the request.
		 * @param server The server to which the CTCP request will be sent
		 * @param client The identifier of the client from which the CTCP request will be sent
		 * @param channel The channel to which the CTCP request will be sent
		 * @param status The status char to which the CTCP request will be sent, e.g. '@' if the request will be sent to @#channel; if
		 * none, it is a blank space (' ')
		 * @param ctcp The type of CTCP request ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP request
		 */
		virtual void onChanCTCPOut(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params);
		
		/** Sent channel CTCP hook
		 * This function is called when a channel CTCP request has been sent to the server, allowing modules to react.
		 * @param server The server to which the CTCP request was sent
		 * @param client The identifier of the client from which the CTCP request was sent
		 * @param channel The channel to which the CTCP request was sent
		 * @param status The status char to which the CTCP request was sent, e.g. '@' if the request was sent to @#channel; if none, it
		 * is a blank space (' ')
		 * @param ctcp The type of CTCP request ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP request
		 */
		virtual void onChanCTCPSend(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		
		/** Outgoing user CTCP hook
		 * This function is called when a user CTCP request is about to be sent to the server, allowing modules to modify the request.
		 * @param server The server to which the CTCP request will be sent
		 * @param client The identifier of the client from whch the CTCP request will be sent
		 * @param nick The nick of the user to whom the CTCP request will be sent
		 * @param ctcp The type of CTCP request ("PING" "VERSION" etc.)
		 * @param param Any data associated with the CTCP request
		 */
		virtual void onUserCTCPOut(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params);
		
		/** Sent user CTCP hook
		 * This function is called when a user CTCP request has been sent to the server, allowing modules to react.
		 * @param server The server to which the CTCP request was sent
		 * @param client The identifier of the client from which the CTCP request was sent
		 * @param nick The nick of the user to whom the CTCP request was sent
		 * @param ctcp The type of CTCP request ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP request
		 */
		virtual void onUserCTCPSend(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		/** Outgoing channel CTCP reply hook
		 * This function is called when a channel CTCP reply is about to be sent to the server, allowing modules to modify the reply.
		 * @param server The server to which the CTCP reply will be sent
		 * @param client The identifier of the client from which the CTCP reply will be sent
		 * @param channel The channel to whcih the CTCP reply will be sent
		 * @param status The status char to which the CTCP reply will be sent, e.g. '@' if the CTCP reply will be sent to @#channel; if
		 * none, it is a blank space (' ')
		 * @param ctcp The type of CTCP reply ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP reply
		 */
		virtual void onChanCTCPReplyOut(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params);
		
		/** Sent channel CTCP reply hook
		 * This function is called when a channel CTCP reply has been sent to the server, allowing modules to react.
		 * @param server The server to which the CTCP reply has been sent
		 * @param client The identifier of the client that sent the CTCP reply
		 * @param channel The channel to which the CTCP reply was sent
		 * @param status The status char to which the CTCP reply was sent, e.g. '@' if the CTCP reply was sent to @#channel; if none, it
		 * is a blank space (' ')
		 * @param ctcp The type of CTCP reply ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP reply
		 */
		virtual void onChanCTCPReplySend(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		
		/** Outgoing user CTCP reply hook
		 * This function is called when a user CTCP reply is about to be sent to the server, allowing modules to modify the reply.
		 * @param server The server to which the CTCP reply will be sent
		 * @param client The identifier of the client from which the CTCP reply will be sent
		 * @param nick The nick of the user to whom the CTCP reply will be sent
		 * @param ctcp The type of CTCP reply ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP reply
		 */
		virtual void onUserCTCPReplyOut(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params);
		
		/** Sent user CTCP reply hook
		 * This function is called when a user CTCP reply has been sent to the server, allowing modules to react.
		 * @param server The server to which the CTCP reply was sent
		 * @param client The identifier of the client that sent the CTCP reply
		 * @param nick The nick of the user to whom the CTCP reply was sent
		 * @param ctcp The type of CTCP reply ("PING" "VERSION" etc.)
		 * @param params Any data associated with the CTCP reply
		 */
		virtual void onUserCTCPReplySend(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		
		/** Description
		 * Allows modules to provide a human-readable description of their function.
		 * @return A human-readable description of the module
		 */
		virtual std::string description();
		
		/** Provided services
		 * Allows the bot core to determine which services this module provides.
		 * @return A list of services provided by the module
		 */
		virtual std::list<std::string> provides();
		
		/** Required services
		 * Allows the bot core to determine which services must be provided by other modules for this module to function.
		 * @return A list of services required by the module
		 */
		virtual std::list<std::string> requires();
		
		/** Supported services
		 * Allows the bot core to determine which services this module makes use of but does not require.
		 * @return A list of services used by the module
		 */
		virtual std::list<std::string> supports();
	protected:
		/** Utility variables
		 * moduleName: The name the bot core uses to reference this module
		 * workingDir: The directory of the bot relative to where it was started from
		 */
		const std::string moduleName, workingDir;
		
		/// Holds the module configuration as provided in the configuration file
		std::map<std::string, std::string> config;
		
		/// The current debug level of the bot, used to determine how much output to give
		unsigned short debugLevel;
		
		
		/** Send message
		 * Used by modules to send a message.
		 * @param server The server to send the PRIVMSG to
		 * @param client The identifier of the client to send the PRIVMSG
		 * @param target The destination of the message
		 * @param message The message to send
		 */
		void sendPrivMsg(std::string server, std::string client, std::string target, std::string message);
		
		/** Send notice
		 * Used by modules to send a notice.
		 * @param server The server to send the notice to
		 * @param client The identifier of the client to send the notice
		 * @param target The notice's destination
		 * @param message The message of the notice to send
		 */
		void sendNotice(std::string server, std::string client, std::string target, std::string message);
		
		/** Send CTCP request
		 * Used by modules to send a CTCP request.
		 * @param server The server to send the CTCP request to
		 * @param client The identifier of the client to send the CTCP request
		 * @param target The destination of the CTCP request
		 * @param ctcp The type of CTCP request
		 * @param params Any data associated with the CTCP request
		 */
		void sendCTCP(std::string server, std::string client, std::string target, std::string ctcp, std::string params = "");
		
		/** Send CTCP reply
		 * Used by modules to send a CTCP reply.
		 * @param server The server to send the CTCP reply to
		 * @param client The identifier of the client to send the CTCP reply
		 * @param target The destination of the CTCP reply
		 * @param ctcp The type of CTCP reply
		 * @param params Any data associated with the CTCP reply
		 */
		void sendCTCPReply(std::string server, std::string client, std::string target, std::string ctcp, std::string params = "");
		
		/** Set mode
		 * Used by modules to set modes.
		 * @param server The server on which to set a mode
		 * @param client The identifier of the client to set the mode
		 * @param target The channel (or user, if user mode) on which to set the mode
		 * @param setModes A list of long-name modes to set on the channel (or empty list if none)
		 * @param delModes A list of long-name modes to unset on the channel (or empty list if none)
		 */
		void setMode(std::string server, std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes);
		
		/** Set SNOmask
		 * Used by modules to set SNOmasks.
		 * @param server The server on which to set a SNOmask
		 * @param client The identifier of the client on which to set the SNOmask
		 * @param snomask The SNOmask type to set or unset
		 * @param add Whether to set (true) or unset (false) the SNOmask char in question
		 */
		void setSNOMask(std::string server, std::string client, char snomask, bool add);
		
		/** Set channel topic
		 * Used by modules to set the channel topic
		 * @param server The server on which to change the topic
		 * @param client The identifier of the client to change the topic
		 * @param channel The channel whose topic should be changed
		 * @param topic The new channel topic
		 */
		void setChanTopic(std::string server, std::string client, std::string channel, std::string topic);
		
		/** Join channel
		 * Used by modules to have the bot join a channel
		 * @param server The server on which to join a channel
		 * @param client The identifier of the client to join a channel
		 * @param channel The channel to join
		 * @param key The channel key
		 */
		void joinChannel(std::string server, std::string client, std::string channel, std::string key = "");
		
		/** Part channel
		 * Used by modules to have the bot part a channel
		 * @param server The server on which to part a channel
		 * @param client The identifier of the client to part a channel
		 * @param channel The channel to part
		 * @param reason The part reason
		 */
		void partChannel(std::string server, std::string client, std::string channel, std::string reason);
		
		/** Change nick
		 * Used by modules to change the nick of a bot client
		 * @param server The server on which to change nicks
		 * @param client The identifier of the client to change nicks
		 * @param newNick The client's new nick
		 */
		void changeNick(std::string server, std::string client, std::string newNick);
		
		/** Kick user
		 * Used by modules to kick a user from a channel
		 * @param server The server on which to kick a user
		 * @param client The identifier of the client to perform the kicking
		 * @param channel The channel in which to kick a user
		 * @param nick The nick of the user to kick
		 * @param reason The kick reason
		 */
		void kickUser(std::string server, std::string client, std::string channel, std::string nick, std::string reason);
		
		/** Connect server
		 * Used by modules to connect to a new server specified in the config
		 * @param server The server to connect to
		 */
		void connectServer(std::string server);
		
		/** Add new client
		 * Used by modules to introduce a new client to an existing server
		 * @param server The server on which to add a client
		 * @param nick The nick of the new client
		 * @param ident The ident of the new client
		 * @param host The host of the new client
		 * @param gecos The gecos of the new client
		 * @return An identifier that refers to the newly-introduced client
		 */
		std::string addClient(std::string server, std::string nick, std::string ident, std::string host, std::string gecos);
		
		/** Remove client
		 * Used by modules to remove a bot client from a server
		 * @param server The server from which to remove a client
		 * @param client The identifier of the client to remove
		 */
		void removeClient(std::string server, std::string client);
		
		/** Quit server
		 * Used by modules to entirely disconnect from a server.
		 * @param server The server from which to disconnect
		 */
		void quitServer(std::string server);
		
		/** Oper
		 * Used by modules to oper up
		 * @param server The server on which to oper up
		 * @param client The identifier of the client to oper up
		 * @param username The username to use for opering up (or other things e.g. opertype, depending on the protocol module)
		 * @param password The password to use for opering up (or other things, depending on the protocol module)
		 */
		void oper(std::string server, std::string client, std::string username, std::string password);
		
		/** Send SNOtice
		 * Used by modules to send a server notice
		 * @param server The server on which to send a server notice
		 * @param snomask The snomask type to send the message under
		 * @param message The message to send
		 */
		void sendSNotice(std::string server, char snomask, std::string message);
		
		/** Set metadata
		 * Used by modules to set metadata on a user or channel.
		 * @param server The server on which to set metadata
		 * @param target The user or channel on which to set metadata
		 * @param key The data key to change
		 * @param value The new value for the metadata
		 */
		void setMetadata(std::string server, std::string target, std::string key, std::string value);
		
		/** Set x:line
		 * Used by modules to set an x:line.
		 * @param server The server on which to set an x:line
		 * @param client The identifier of the client to set the x:line
		 * @param lineType The type of x:line to set
		 * @param mask The mask on which to set the x:line
		 * @param duration The desired duration of the x:line
		 * @param reason The x:line reason
		 */
		void setXLine(std::string server, std::string client, std::string linetype, std::string mask, time_t duration, std::string reason);
		
		/** Remove x:line
		 * Used by modules to remove an x:line
		 * @param server The server on which to remove the x:line
		 * @param client The identifier of the client to remove the x:line
		 * @param lineType The type of x:line to remove
		 * @param mask The mask from which to remove the x:line
		 */
		void delXLine(std::string server, std::string client, std::string linetype, std::string mask);
		
		/** Send other data
		 * Used by modules to send a line to the server that cannot be sent by other functions in the API.
		 * @param server The server to which to send the data
		 * @param client The identifier of the client to send the data
		 * @param line The line to send
		 */
		void sendOtherData(std::string server, std::string client, std::string line);
	private:
		/// Holds a pointer to the bot base for communication with protocol modules and other normal modules
		Base* bot;
};