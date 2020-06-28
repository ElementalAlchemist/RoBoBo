use crate::irc_data::{Channel, User};
use std::collections::HashMap;
use std::default::Default;
use std::hash::Hash;

pub struct ModuleInvokeData<'a> {
	invoke_type: &'static str,
	tagged_users: Option<HashMap<&'static str, &'a User>>,
	tagged_channels: Option<HashMap<&'static str, &'a Channel>>,
	tagged_user_list: Option<HashMap<&'static str, Vec<&'a User>>>,
	tagged_channel_list: Option<HashMap<&'static str, Vec<&'a Channel>>>,
	extra_string_data: Option<HashMap<&'static str, String>>,
	extra_numeric_data: Option<HashMap<&'static str, i32>>,
	extra_float_numeric_data: Option<HashMap<&'static str, f64>>,
	extra_bool_data: Option<HashMap<&'static str, bool>>,
}

impl<'a> ModuleInvokeData<'a> {
	pub fn new(invoke_type: &'static str) -> ModuleInvokeData<'a> {
		ModuleInvokeData {
			invoke_type,
			tagged_users: None,
			tagged_channels: None,
			tagged_user_list: None,
			tagged_channel_list: None,
			extra_string_data: None,
			extra_numeric_data: None,
			extra_float_numeric_data: None,
			extra_bool_data: None,
		}
	}

	pub fn set_tagged_user(&'a mut self, tag: &'static str, user: &'a User) {
		self.tagged_users = set_value_in_option_hashmap(self.tagged_users.take(), tag, user);
	}

	pub fn set_tagged_channel(&'a mut self, tag: &'static str, channel: &'a Channel) {
		self.tagged_channels = set_value_in_option_hashmap(self.tagged_channels.take(), tag, channel);
	}

	pub fn add_user_to_tag_list(&'a mut self, tag: &'static str, user: &'a User) {
		self.tagged_user_list = add_value_to_option_hashmap_list(self.tagged_user_list.take(), tag, user);
	}

	pub fn add_channel_to_tag_list(&'a mut self, tag: &'static str, channel: &'a Channel) {
		self.tagged_channel_list = add_value_to_option_hashmap_list(self.tagged_channel_list.take(), tag, channel);
	}

	pub fn set_string_value(&'a mut self, name: &'static str, value: String) {
		self.extra_string_data = set_value_in_option_hashmap(self.extra_string_data.take(), name, value);
	}

	pub fn set_number_value(&'a mut self, name: &'static str, value: i32) {
		self.extra_numeric_data = set_value_in_option_hashmap(self.extra_numeric_data.take(), name, value);
	}

	pub fn set_float_value(&'a mut self, name: &'static str, value: f64) {
		self.extra_float_numeric_data = set_value_in_option_hashmap(self.extra_float_numeric_data.take(), name, value);
	}

	pub fn set_bool_value(&'a mut self, name: &'static str, value: bool) {
		self.extra_bool_data = set_value_in_option_hashmap(self.extra_bool_data.take(), name, value);
	}

	pub fn get_invoke_type(&self) -> &'static str {
		self.invoke_type
	}

	pub fn get_user_from_tag(&self, tag: &'static str) -> Option<&'a User> {
		if let Some(user_map) = self.tagged_users.as_ref() {
			if let Some(user) = user_map.get(tag) {
				return Some(*user);
			}
		}
		None
	}

	pub fn get_channel_from_tag(&self, tag: &'static str) -> Option<&'a Channel> {
		if let Some(channel_map) = self.tagged_channels.as_ref() {
			if let Some(channel) = channel_map.get(tag) {
				return Some(*channel);
			}
		}
		None
	}

	pub fn get_user_list_from_tag(&self, tag: &'static str) -> Option<&Vec<&'a User>> {
		if let Some(user_list_map) = self.tagged_user_list.as_ref() {
			if let Some(user_list) = user_list_map.get(tag) {
				return Some(user_list);
			}
		}
		None
	}

	pub fn get_channel_list_from_tag(&self, tag: &'static str) -> Option<&Vec<&'a Channel>> {
		if let Some(channel_list_map) = self.tagged_channel_list.as_ref() {
			if let Some(channel_list) = channel_list_map.get(tag) {
				return Some(channel_list);
			}
		}
		None
	}

	pub fn get_string_value(&self, name: &'static str) -> Option<&String> {
		if let Some(data_map) = self.extra_string_data.as_ref() {
			if let Some(value) = data_map.get(name) {
				return Some(value);
			}
		}
		None
	}

	pub fn get_number_value(&self, name: &'static str) -> Option<i32> {
		if let Some(data_map) = self.extra_numeric_data.as_ref() {
			if let Some(value) = data_map.get(name) {
				return Some(*value);
			}
		}
		None
	}

	pub fn get_float_value(&self, name: &'static str) -> Option<f64> {
		if let Some(data_map) = self.extra_float_numeric_data.as_ref() {
			if let Some(value) = data_map.get(name) {
				return Some(*value);
			}
		}
		None
	}

	pub fn get_bool_value(&self, name: &'static str) -> Option<bool> {
		if let Some(data_map) = self.extra_bool_data.as_ref() {
			if let Some(value) = data_map.get(name) {
				return Some(*value);
			}
		}
		None
	}
}

fn set_value_in_option_hashmap<K, V>(map_option: Option<HashMap<K, V>>, key: K, value: V) -> Option<HashMap<K, V>>
where
	HashMap<K, V>: Default,
	K: Eq + Hash,
{
	let mut map = map_option.unwrap_or_default();
	map.insert(key, value);
	Some(map)
}

fn add_value_to_option_hashmap_list<K, V>(
	map_option: Option<HashMap<K, Vec<V>>>,
	key: K,
	value: V,
) -> Option<HashMap<K, Vec<V>>>
where
	HashMap<K, Vec<V>>: Default,
	K: Eq + Hash,
{
	let mut map = map_option.unwrap_or_default();
	let list = map.entry(key).or_default();
	list.push(value);
	Some(map)
}

#[cfg(test)]
mod tests {
	use super::*;

	#[test]
	fn module_invoke_data_created() -> Result<(), &'static str> {
		let data = ModuleInvokeData::new("test");
		if data.get_invoke_type() == "test" {
			Ok(())
		} else {
			Err("The module invoke data didn't get the correct type")
		}
	}

	#[test]
	fn value_set_in_option_hashmap_none() -> Result<(), &'static str> {
		let mut map_option: Option<HashMap<&'static str, &'static str>> = None;
		map_option = set_value_in_option_hashmap(map_option, "test", "one");
		if map_option.is_none() {
			return Err("Map was not created");
		}
		let map = map_option.unwrap();
		if !map.contains_key("test") {
			return Err("Key was not added to map");
		}
		if map["test"] != "one" {
			return Err("Value of key \"test\" is not correct");
		}
		Ok(())
	}

	#[test]
	fn value_set_in_option_hashmap_empty() -> Result<(), &'static str> {
		let mut map_option: Option<HashMap<&'static str, &'static str>> = Some(HashMap::new());
		map_option = set_value_in_option_hashmap(map_option, "test", "one");
		if map_option.is_none() {
			return Err("Map became None");
		}
		let map = map_option.unwrap();
		if !map.contains_key("test") {
			return Err("Key was not added to map");
		}
		if map["test"] != "one" {
			return Err("Value of key \"test\" is not correct");
		}
		Ok(())
	}

	#[test]
	fn value_set_in_option_hashmap_new_key() -> Result<(), &'static str> {
		let mut existing_map: HashMap<&'static str, &'static str> = HashMap::new();
		existing_map.insert("butts", "yes");
		let mut map_option: Option<HashMap<&'static str, &'static str>> = Some(existing_map);
		map_option = set_value_in_option_hashmap(map_option, "test", "one");
		if map_option.is_none() {
			return Err("Map became None");
		}
		let map = map_option.unwrap();
		if !map.contains_key("test") {
			return Err("Key was not added to map");
		}
		if !map.contains_key("butts") {
			return Err("Key was lost from map");
		}
		if map["test"] != "one" {
			return Err("Value of key \"test\" is not correct");
		}
		if map["butts"] != "yes" {
			return Err("Value of key \"yes\" is not correct");
		}
		Ok(())
	}

	#[test]
	fn value_set_in_option_hashmap_existing() -> Result<(), &'static str> {
		let mut existing_map: HashMap<&'static str, &'static str> = HashMap::new();
		existing_map.insert("test", "one");
		let mut map_option: Option<HashMap<&'static str, &'static str>> = Some(existing_map);
		map_option = set_value_in_option_hashmap(map_option, "test", "two");
		if map_option.is_none() {
			return Err("Map became None");
		}
		let map = map_option.unwrap();
		if !map.contains_key("test") {
			return Err("Key was removed from map");
		}
		if map["test"] == "one" {
			return Err("Value of key \"test\" was not updated");
		}
		if map["test"] != "two" {
			return Err("Value of key \"test\" is not correct");
		}
		Ok(())
	}

	#[test]
	fn value_added_to_option_hashmap_list_none() -> Result<(), &'static str> {
		let mut map_option: Option<HashMap<&'static str, Vec<&'static str>>> = None;
		map_option = add_value_to_option_hashmap_list(map_option, "test", "one");
		if map_option.is_none() {
			return Err("Map was not created");
		}
		let map = map_option.unwrap();
		if !map.contains_key("test") {
			return Err("Key was not added to map");
		}
		let list = &map["test"];
		if list.len() != 1 {
			return Err("List should have exactly 1 item in it, but it does not");
		}
		if list[0] != "one" {
			return Err("The list value at index 0 is incorrect");
		}
		Ok(())
	}

	#[test]
	fn value_added_to_option_hashmap_list_empty() -> Result<(), &'static str> {
		let mut map_option: Option<HashMap<&'static str, Vec<&'static str>>> = Some(HashMap::new());
		map_option = add_value_to_option_hashmap_list(map_option, "test", "one");
		if map_option.is_none() {
			return Err("Map became None");
		}
		let map = map_option.unwrap();
		if !map.contains_key("test") {
			return Err("Key was not added to map");
		}
		let list = &map["test"];
		if list.len() != 1 {
			return Err("List should have exactly 1 item in it, but it does not");
		}
		if list[0] != "one" {
			return Err("The list value at index 0 is incorrect");
		}
		Ok(())
	}

	#[test]
	fn value_added_to_option_hashmap_list_new_key() -> Result<(), &'static str> {
		let mut existing_map: HashMap<&'static str, Vec<&'static str>> = HashMap::new();
		existing_map.insert("butts", vec!["yes", "obviously"]);
		let mut map_option: Option<HashMap<&'static str, Vec<&'static str>>> = Some(existing_map);
		map_option = add_value_to_option_hashmap_list(map_option, "test", "one");
		if map_option.is_none() {
			return Err("Map became None");
		}
		let map = map_option.unwrap();
		if !map.contains_key("butts") {
			return Err("The existing key was lost");
		}
		let butts_list = &map["butts"];
		if butts_list.len() != 2 {
			return Err("Existing list should have exactly 2 items, but it does not");
		}
		if butts_list[0] != "yes" || butts_list[1] != "obviously" {
			return Err("Existing list values have been changed");
		}
		if !map.contains_key("test") {
			return Err("Key was not added to map");
		}
		let list = &map["test"];
		if list.len() != 1 {
			return Err("List should have exactly 1 item in it, but it does not");
		}
		if list[0] != "one" {
			return Err("The list value at index 0 is incorrect");
		}
		Ok(())
	}

	#[test]
	fn value_added_to_option_hashmap_list_new_key_multiple() -> Result<(), &'static str> {
		let mut existing_map: HashMap<&'static str, Vec<&'static str>> = HashMap::new();
		existing_map.insert("butts", vec!["yes", "obviously"]);
		let mut map_option: Option<HashMap<&'static str, Vec<&'static str>>> = Some(existing_map);
		map_option = add_value_to_option_hashmap_list(map_option, "test", "one");
		map_option = add_value_to_option_hashmap_list(map_option, "test", "two");
		if map_option.is_none() {
			return Err("Map became None");
		}
		let map = map_option.unwrap();
		if !map.contains_key("butts") {
			return Err("The existing key was lost");
		}
		let butts_list = &map["butts"];
		if butts_list.len() != 2 {
			return Err("Existing list should have exactly 2 items, but it does not");
		}
		if butts_list[0] != "yes" || butts_list[1] != "obviously" {
			return Err("Existing list values have been changed");
		}
		if !map.contains_key("test") {
			return Err("Key was not added to map");
		}
		let list = &map["test"];
		if list.len() != 2 {
			return Err("List should have exactly 2 items in it, but it does not");
		}
		if list[0] != "one" {
			return Err("The list value at index 0 is incorrect");
		}
		if list[1] != "two" {
			return Err("The list value at index 1 is incorrect");
		}
		Ok(())
	}

	#[test]
	fn value_added_to_option_hashmap_list_existing() -> Result<(), &'static str> {
		let mut existing_map: HashMap<&'static str, Vec<&'static str>> = HashMap::new();
		existing_map.insert("test", vec!["butts"]);
		let mut map_option: Option<HashMap<&'static str, Vec<&'static str>>> = Some(existing_map);
		map_option = add_value_to_option_hashmap_list(map_option, "test", "one");
		if map_option.is_none() {
			return Err("Map became None");
		}
		let map = map_option.unwrap();
		if !map.contains_key("test") {
			return Err("The key was lost");
		}
		let list = &map["test"];
		if list.len() != 2 {
			return Err("List should have exactly 2 items, but it does not");
		}
		if list[0] != "butts" {
			return Err("The existing list value at index 0 is incorrect");
		}
		if list[1] != "one" {
			return Err("The new value at index 1 is incorrect");
		}
		Ok(())
	}
}