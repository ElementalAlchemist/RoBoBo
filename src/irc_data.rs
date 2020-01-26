use chrono::{DateTime, Utc};
use std::collections::HashMap;
use std::sync::Weak;

pub struct User {
	id: String,
	nick: String,
	ident: String,
	host: String,
	gecos: String,
	channels: Vec<Weak<Channel>>,
}

pub struct Channel {
	name: String,
	topic: String,
	topic_setter: String,
	topic_time: DateTime<Utc>,
	users: Vec<ChannelUser>,
	list_modes: HashMap<char, Vec<String>>,
	modes: HashMap<char, Option<String>>,
}

pub struct ChannelUser {
	user: Weak<User>,
	rank: char,
}
