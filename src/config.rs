use std::collections::HashMap;
use std::fmt;
use std::fs;
use std::io;
use std::io::prelude::*;

/// Represents configuration data. The functionality that can be configured falls into two categories,
/// each represented by a callable function returning data.
pub struct Config {
	modules: HashMap<String, HashMap<String, String>>,
	connections: HashMap<String, ConnectionData>,
}

impl Config {
	/// Returns a HashMap of all module configuration. For each module, a key-value pair of
	/// configuration values is provided.
	pub fn get_module_data(&self) -> &HashMap<String, HashMap<String, String>> {
		&self.modules
	}

	/// Returns a HashMap of all connections the bot can make. For each named connection, a
	/// connection type and a key-value pair of configuration values are provided.
	pub fn get_connection_data(&self) -> &HashMap<String, ConnectionData> {
		&self.connections
	}
}
/*
Config blocks:

declare {
	var1 = "butts";
	var2 = var1 + " yes";
}

include "butts/poop.conf"

module ExtraFunctionality {
	extraval = "5";
	autoload = "true";
}

connection SomeNetwork client plain {
	server = "127.0.0.1";
	port = "6667";
	autoconnect = "true";
}
*/

/// Represents the connection data for a single connection that is configured in the bot.
pub struct ConnectionData {
	connection_protocol: String,
	connection_socket: String,
	connection_data: HashMap<String, String>,
}

impl ConnectionData {
	/// Returns the protocol type name of the connection to make
	pub fn get_protocol(&self) -> &String {
		&self.connection_protocol
	}

	/// Returns the socket type name of the connection to make
	pub fn get_socket(&self) -> &String {
		&self.connection_socket
	}

	/// Returns the key-value pair configuration data for a connection
	pub fn get_data(&self) -> &HashMap<String, String> {
		&self.connection_data
	}
}

/// Represents a file I/O error or parse error that occurred reading configuration data
pub enum ConfigError {
	FileError(io::Error),
	ParseError(ConfigParseError),
}

pub struct ConfigParseError {
	file_name: String,
	line_number: u32,
	message: String,
}

impl fmt::Display for ConfigParseError {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(
			f,
			"An error occurred parsing configuration file {} on line {}: {}",
			self.file_name, self.line_number, self.message
		)
	}
}

/// Reads and parses the bot configuration.
/// Pass the file name of the primary configuration file. This function
/// will read all blocks and follow all includes to read the entire configuration
/// file.
///
/// On success. returns all parsed configuration data.
/// On failure, returns a configuration error representing an I/O error or a
/// parsing failure.
pub fn read_config(file_name: &str) -> Result<Config, ConfigError> {
	let mut file_name = file_name;
	if file_name == "" {
		file_name = "robobo.conf";
	}

	Ok(read_config_file(&file_name, &mut HashMap::new())?)
}

/// Given a string value, attempts to parse it as a boolean. If the string
/// value can be parsed as a boolean value, returns the bool representation;
/// otherwise, returns None.
pub fn parse_config_value_as_bool(value: &str) -> Option<bool> {
	if value == "1" {
		return Some(true);
	}
	if value == "0" {
		return Some(false);
	}
	let lower_value = value.to_lowercase();
	if lower_value == "true" {
		return Some(true);
	}
	if lower_value == "false" {
		return Some(false);
	}
	if lower_value == "yes" {
		return Some(true);
	}
	if lower_value == "no" {
		return Some(false);
	}
	None
}

enum ParseToken {
	Variable(String),
	Literal(String),
}

enum ParseExpectOperation {
	Variable,
	AssignOperator,
	Expression,
	ValueOperator,
}

fn read_config_file(file_name: &str, declared_variables: &mut HashMap<String, String>) -> Result<Config, ConfigError> {
	let file = match fs::File::open(&file_name) {
		Ok(f) => f,
		Err(e) => return Err(ConfigError::FileError(e)),
	};

	let mut line_number = 1;

	let mut buffer: Vec<u8> = Vec::new();
	let mut blocks: Vec<(String, u32)> = Vec::new();
	for current_byte in file.bytes() {
		let current_byte = match current_byte {
			Ok(b) => b,
			Err(e) => return Err(ConfigError::FileError(e)),
		};
		if current_byte == 10 {
			// \n
			line_number += 1;
		}
		buffer.push(current_byte);
		if current_byte == 125 {
			// }
			blocks.push((String::from_utf8_lossy(&buffer).to_string(), line_number));
			buffer.clear();
		}
	}

	let mut declare_blocks = Vec::new();
	let mut include_blocks = Vec::new();
	let mut module_blocks = Vec::new();
	let mut connection_blocks = Vec::new();

	let mut modules: HashMap<String, HashMap<String, String>> = HashMap::new();
	let mut connections: HashMap<String, ConnectionData> = HashMap::new();

	for (block, line) in blocks {
		if block.starts_with("declare ") || block.starts_with("declare\n") {
			declare_blocks.push((String::from(block[8..].trim()), line));
		} else if block.starts_with("include ") || block.starts_with("include\n") {
			include_blocks.push((String::from(block[8..].trim()), line));
		} else if block.starts_with("module ") || block.starts_with("module\n") {
			module_blocks.push((String::from(block[7..].trim()), line));
		} else if block.starts_with("connection ") || block.starts_with("connection\n") {
			connection_blocks.push((String::from(block[11..].trim()), line));
		} else {
			return Err(ConfigError::ParseError(ConfigParseError {
				file_name: String::from(file_name),
				line_number: line,
				message: format!("Unrecognized block type: {}", block),
			}));
		}
	}

	for (block, line) in declare_blocks {
		match parse_declare_instruction(&block, declared_variables, file_name, line) {
			Ok(mut result) => {
				for (var, value) in result.drain() {
					declared_variables.insert(var, value);
				}
			}
			Err(e) => return Err(ConfigError::ParseError(e)),
		}
	}

	for (block, line) in include_blocks {
		let mut result = parse_include_instruction(&block, declared_variables, file_name, line)?;
		for (module, data) in result.modules.drain() {
			modules.insert(module, data);
		}
		for (connection, data) in result.connections.drain() {
			connections.insert(connection, data);
		}
	}

	for (block, line) in module_blocks {
		match parse_module_instruction(&block, declared_variables, file_name, line) {
			Ok((name, data)) => {
				if modules.contains_key(&name) {
					return Err(ConfigError::ParseError(ConfigParseError {
						file_name: String::from(file_name),
						line_number: line,
						message: format!("Redeclared module {}", name),
					}));
				}
				modules.insert(name, data);
			}
			Err(e) => return Err(ConfigError::ParseError(e)),
		}
	}

	for (block, line) in connection_blocks {
		match parse_connection_instruction(&block, declared_variables, file_name, line) {
			Ok((name, data)) => {
				if connections.contains_key(&name) {
					return Err(ConfigError::ParseError(ConfigParseError {
						file_name: String::from(file_name),
						line_number: line,
						message: format!("Redeclared connection {}", name),
					}));
				}
				connections.insert(name, data);
			}
			Err(e) => return Err(ConfigError::ParseError(e)),
		}
	}

	Ok(Config { modules, connections })
}

fn parse_declare_instruction(
	declare_block: &str,
	variables: &HashMap<String, String>,
	file_name: &str,
	line_number: u32,
) -> Result<HashMap<String, String>, ConfigParseError> {
	if !declare_block.starts_with("{") || !declare_block.ends_with("}") {
		return Err(ConfigParseError {
			file_name: String::from(file_name),
			line_number,
			message: String::from("Declare must only be a single block"),
		});
	}

	let declare_block = &declare_block[1..declare_block.len() - 1];
	let mut buffer = String::new();
	let mut current_variable = String::new();
	let mut concat_value: Vec<ParseToken> = Vec::new();
	let mut expecting = ParseExpectOperation::Variable;
	let mut in_string_literal = false;
	let mut escaping_character = false;
	let mut new_variables: HashMap<String, Vec<ParseToken>> = HashMap::new();

	for current_char in declare_block.chars() {
		match expecting {
			ParseExpectOperation::Variable => {
				if current_char == ';' {
					return Err(ConfigParseError {
						file_name: String::from(file_name),
						line_number,
						message: String::from("Unexpected end of variable declaration"),
					});
				}
				if current_char.is_whitespace() {
					if !buffer.is_empty() {
						current_variable = buffer.drain(..).collect();
						expecting = ParseExpectOperation::AssignOperator;
					}
				} else if current_char == '=' {
					if buffer.is_empty() {
						return Err(ConfigParseError {
							file_name: String::from(file_name),
							line_number,
							message: String::from("Expected variable name, not ="),
						});
					}
					current_variable = buffer.drain(..).collect();
					expecting = ParseExpectOperation::Expression;
				} else {
					buffer.push(current_char);
				}
			}
			ParseExpectOperation::AssignOperator => {
				if current_char.is_whitespace() {
					continue;
				}
				if current_char == '=' {
					expecting = ParseExpectOperation::Expression;
				} else {
					return Err(ConfigParseError {
						file_name: String::from(file_name),
						line_number,
						message: String::from("Expected assignment operation"),
					});
				}
			}
			ParseExpectOperation::Expression => {
				if current_char == '"' {
					if escaping_character {
						buffer.push(current_char);
					} else {
						if in_string_literal {
							in_string_literal = false;
							expecting = ParseExpectOperation::ValueOperator;
							concat_value.push(ParseToken::Literal(buffer.drain(..).collect()));
						} else {
							if !buffer.is_empty() {
								return Err(ConfigParseError {
									file_name: String::from(file_name),
									line_number,
									message: String::from("Unexpected start of string"),
								});
							}
							in_string_literal = true;
						}
					}
				} else if current_char == '\\' && in_string_literal {
					if escaping_character {
						buffer.push(current_char);
					} else {
						escaping_character = true;
					}
				} else if current_char == ';' && !in_string_literal {
					if buffer.is_empty() {
						return Err(ConfigParseError {
							file_name: String::from(file_name),
							line_number,
							message: String::from("Unexpected end of statement; expected value"),
						});
					}
					concat_value.push(ParseToken::Variable(buffer.drain(..).collect()));

					if new_variables.contains_key(&current_variable) {
						return Err(ConfigParseError {
							file_name: String::from(file_name),
							line_number,
							message: String::from("Duplicate variable declaration"),
						});
					}
					new_variables.insert(current_variable.drain(..).collect(), concat_value.drain(..).collect());
					expecting = ParseExpectOperation::Variable;
				} else if !in_string_literal && current_char.is_whitespace() {
					if !buffer.is_empty() {
						concat_value.push(ParseToken::Variable(buffer.drain(..).collect()));
						expecting = ParseExpectOperation::ValueOperator;
					}
					continue;
				} else {
					buffer.push(current_char);
				}
			}
			ParseExpectOperation::ValueOperator => {
				if current_char == ';' {
					if new_variables.contains_key(&current_variable) {
						return Err(ConfigParseError {
							file_name: String::from(file_name),
							line_number,
							message: String::from("Duplicate variable definition"),
						});
					}
					new_variables.insert(current_variable.drain(..).collect(), concat_value.drain(..).collect());
					expecting = ParseExpectOperation::Variable;
					continue;
				}
				if current_char == '+' {
					expecting = ParseExpectOperation::Expression;
				} else if current_char != ' ' && current_char != '\n' {
					return Err(ConfigParseError {
						file_name: String::from(file_name),
						line_number,
						message: String::from("Expected operator"),
					});
				}
			}
		}
	}

	let mut undefined_variables: Vec<&String> = new_variables.keys().collect();
	let mut new_variable_values: HashMap<String, String> = HashMap::new();

	while !undefined_variables.is_empty() {
		let mut completed_variables: Vec<&String> = Vec::new();

		for variable in undefined_variables.iter() {
			let mut value = String::new();
			let mut unresolved_dependency_in_value = false;

			for value_token in new_variables[*variable].iter() {
				match value_token {
					ParseToken::Literal(val) => value.push_str(&val),
					ParseToken::Variable(var) => {
						if variables.contains_key(var) {
							value.push_str(&variables[var]);
						} else if new_variables.contains_key(var) {
							if new_variable_values.contains_key(var) {
								value.push_str(&new_variable_values[var]);
							} else {
								unresolved_dependency_in_value = true;
								break;
							}
						} else {
							return Err(ConfigParseError {
								file_name: String::from(file_name),
								line_number,
								message: format!("Undefined variable used in definition: {}", var),
							});
						}
					}
				}
			}

			if !unresolved_dependency_in_value {
				new_variable_values.insert(String::from(*variable), value);
				completed_variables.push(variable);
			}
		}

		if completed_variables.is_empty() {
			return Err(ConfigParseError {
				file_name: String::from(file_name),
				line_number,
				message: String::from("Circular dependency detected"),
			});
		}
		for complete_variable in completed_variables {
			let mut index_to_remove: Option<usize> = None;
			for (index, remove_variable) in undefined_variables.iter().enumerate() {
				if *remove_variable == complete_variable {
					index_to_remove = Some(index);
					break;
				}
			}
			if let Some(index) = index_to_remove {
				undefined_variables.remove(index);
			}
		}
	}

	Ok(new_variable_values)
}

fn parse_include_instruction(
	include_file: &str,
	variables: &mut HashMap<String, String>,
	file_name: &str,
	line_number: u32,
) -> Result<Config, ConfigError> {
	if !include_file.starts_with("\"") || !include_file.ends_with("\"") {
		return Err(ConfigError::ParseError(ConfigParseError {
			file_name: String::from(file_name),
			line_number,
			message: String::from("Include must be a literal string file path"),
		}));
	}

	let include_file = &include_file[1..include_file.len() - 1];
	let mut path = String::new();
	let mut escaped = false;
	for current_char in include_file.chars() {
		if current_char == '"' && !escaped {
			return Err(ConfigError::ParseError(ConfigParseError {
				file_name: String::from(file_name),
				line_number,
				message: String::from("Include must be a single literal string file path"),
			}));
		}
		if !escaped && current_char == '\\' {
			escaped = true;
			continue;
		}
		escaped = false;
		path.push(current_char);
	}

	return read_config_file(&path, variables);
}

fn parse_module_instruction(
	module_block_data: &str,
	variables: &HashMap<String, String>,
	file_name: &str,
	line_number: u32,
) -> Result<(String, HashMap<String, String>), ConfigParseError> {
	let module_block_data = module_block_data.trim();
	let module_name_end = module_block_data.find(|c: char| c.is_whitespace());
	let module_name = match module_name_end {
		Some(end) => &module_block_data[0..end],
		None => {
			return Err(ConfigParseError {
				file_name: String::from(file_name),
				line_number,
				message: String::from("Module declaration must start with the module name"),
			})
		}
	};

	let module_block_data = &module_block_data[module_name_end.unwrap() + 1..module_block_data.len()].trim();
	Ok((
		module_name.to_string(),
		parse_declare_instruction(module_block_data, variables, file_name, line_number)?,
	))
}

fn parse_connection_instruction(
	connection_block_data: &str,
	variables: &HashMap<String, String>,
	file_name: &str,
	line_number: u32,
) -> Result<(String, ConnectionData), ConfigParseError> {
	let connection_block_data = connection_block_data.trim();
	let network_name_end = connection_block_data.find(|c: char| c.is_whitespace());
	let network_name = match network_name_end {
		Some(end) => &connection_block_data[0..end],
		None => {
			return Err(ConfigParseError {
				file_name: String::from(file_name),
				line_number,
				message: String::from("Connection declaration must start with the network name"),
			})
		}
	};

	let connection_block_data =
		&connection_block_data[network_name_end.unwrap() + 1..connection_block_data.len()].trim();
	let connection_protocol_end = connection_block_data.find(|c: char| c.is_whitespace());
	let connection_protocol = match connection_protocol_end {
		Some(end) => &connection_block_data[0..end],
		None => {
			return Err(ConfigParseError {
				file_name: String::from(file_name),
				line_number,
				message: String::from("The network name must be followed by the connection protocol type"),
			})
		}
	};

	let connection_block_data =
		connection_block_data[connection_protocol_end.unwrap() + 1..connection_block_data.len()].trim();

	let connection_socket_end = connection_block_data.find(|c: char| c.is_whitespace());
	let connection_socket = match connection_socket_end {
		Some(end) => &connection_block_data[0..end],
		None => {
			return Err(ConfigParseError {
				file_name: String::from(file_name),
				line_number,
				message: String::from(
					"The network name and connection protocol type must be followed by the connection socket type",
				),
			})
		}
	};

	let connection_block_data =
		connection_block_data[connection_socket_end.unwrap() + 1..connection_block_data.len()].trim();

	Ok((
		network_name.to_string(),
		ConnectionData {
			connection_protocol: connection_protocol.to_string(),
			connection_socket: connection_socket.to_string(),
			connection_data: parse_declare_instruction(connection_block_data, variables, file_name, line_number)?,
		},
	))
}

#[cfg(test)]
mod tests {
	use super::*;

	fn format_bad_vars_msg(
		bad_vars_msgs: &mut Vec<String>,
		variables: &HashMap<String, String>,
		key_to_check: &str,
		expected_value: &str,
	) {
		if variables[key_to_check] != expected_value {
			bad_vars_msgs.push(format!(
				"Variable `{}` has wrong value \"{}\" (expected \"{}\")",
				key_to_check, variables[key_to_check], expected_value
			));
		}
	}

	#[test]
	fn bool_parse_empty() -> Result<(), &'static str> {
		match parse_config_value_as_bool("") {
			Some(true) => Err("Expected nothing but got true"),
			Some(false) => Err("Expected nothing but got false"),
			None => Ok(()),
		}
	}

	#[test]
	fn bool_parse_one() -> Result<(), &'static str> {
		match parse_config_value_as_bool("1") {
			Some(true) => Ok(()),
			Some(false) => Err("Expected true but got false"),
			None => Err("Expected true but got nothing"),
		}
	}

	#[test]
	fn bool_parse_zero() -> Result<(), &'static str> {
		match parse_config_value_as_bool("0") {
			Some(true) => Err("Expected false but got true"),
			Some(false) => Ok(()),
			None => Err("Expected false but got nothing"),
		}
	}

	#[test]
	fn bool_parse_true() -> Result<(), &'static str> {
		match parse_config_value_as_bool("true") {
			Some(true) => Ok(()),
			Some(false) => Err("Expected true but got false"),
			None => Err("Expected true but got nothing"),
		}
	}

	#[test]
	fn bool_parse_false() -> Result<(), &'static str> {
		match parse_config_value_as_bool("false") {
			Some(true) => Err("Expected false but got true"),
			Some(false) => Ok(()),
			None => Err("Expected false but got nothing"),
		}
	}

	#[test]
	fn bool_parse_yes() -> Result<(), &'static str> {
		match parse_config_value_as_bool("yes") {
			Some(true) => Ok(()),
			Some(false) => Err("Expected true but got false"),
			None => Err("Expected true but got nothing"),
		}
	}

	#[test]
	fn bool_parse_no() -> Result<(), &'static str> {
		match parse_config_value_as_bool("no") {
			Some(true) => Err("Expected false but got true"),
			Some(false) => Ok(()),
			None => Err("Expected false but got nothing"),
		}
	}

	#[test]
	fn bool_parse_two() -> Result<(), &'static str> {
		match parse_config_value_as_bool("2") {
			Some(true) => Err("Expected nothing but got true"),
			Some(false) => Err("Expected nothing but got false"),
			None => Ok(()),
		}
	}

	#[test]
	fn bool_parse_text() -> Result<(), &'static str> {
		match parse_config_value_as_bool("text") {
			Some(true) => Err("Expected nothing but got true"),
			Some(false) => Err("Expected nothing but got false"),
			None => Ok(()),
		}
	}

	#[test]
	fn declare_block_parses_variables() -> Result<(), String> {
		// The leading "declare" keyword should be stripped out first by calling code
		let declare_block = "{
			test = \"1\";
			5=\"five\";
			need_to_test = \"true\";
			fifty_one = \"5\" + test;
		}";
		let predeclared_variables: HashMap<String, String> = HashMap::new();
		let result = parse_declare_instruction(&declare_block, &predeclared_variables, "test", 6);

		match result {
			Ok(vars) => {
				let mut bad_vars_msgs: Vec<String> = Vec::new();
				format_bad_vars_msg(&mut bad_vars_msgs, &vars, "test", "1");
				format_bad_vars_msg(&mut bad_vars_msgs, &vars, "5", "five");
				format_bad_vars_msg(&mut bad_vars_msgs, &vars, "need_to_test", "true");
				format_bad_vars_msg(&mut bad_vars_msgs, &vars, "fifty_one", "51");

				if bad_vars_msgs.is_empty() {
					Ok(())
				} else {
					Err(bad_vars_msgs.join("\n"))
				}
			}
			Err(e) => Err(format!("Failed to parse declare block: {}", e.message)),
		}
	}

	#[test]
	fn declare_block_uses_existing_variables() -> Result<(), String> {
		let declare_block = "{
			test = \"yes\";
			verify = inherit;
			verify_concat_post = inherit + \"4\";
			verify_concat_pre = \"hi \" + inherit;
			verify_concat_both = \"sup \" + inherit + \" ok\";
		}";
		let mut predeclared_variables: HashMap<String, String> = HashMap::new();
		predeclared_variables.insert(String::from("inherit"), String::from("value"));

		let result = parse_declare_instruction(&declare_block, &predeclared_variables, "test", 7);

		match result {
			Ok(vars) => {
				let mut bad_vars_msgs: Vec<String> = Vec::new();
				format_bad_vars_msg(&mut bad_vars_msgs, &vars, "test", "yes");
				format_bad_vars_msg(&mut bad_vars_msgs, &vars, "verify", "value");
				format_bad_vars_msg(&mut bad_vars_msgs, &vars, "verify_concat_post", "value4");
				format_bad_vars_msg(&mut bad_vars_msgs, &vars, "verify_concat_pre", "hi value");
				format_bad_vars_msg(&mut bad_vars_msgs, &vars, "verify_concat_both", "sup value ok");

				if bad_vars_msgs.is_empty() {
					Ok(())
				} else {
					Err(bad_vars_msgs.join("\n"))
				}
			}
			Err(e) => Err(format!("Failed to parse declare block: {}", e.message)),
		}
	}

	#[test]
	fn declare_block_fail_no_braces() -> Result<(), &'static str> {
		let declare_block = "test = \"yes\"";
		let predeclared_variables: HashMap<String, String> = HashMap::new();

		let result = parse_declare_instruction(&declare_block, &predeclared_variables, "test", 1);

		if let Ok(_) = result {
			Err("Successfully parsed invalid block")
		} else {
			Ok(())
		}
	}

	#[test]
	fn declare_block_fail_no_concat_operator() -> Result<(), &'static str> {
		let declare_block = "{
			test = \"4\";
			concat = \"butts\" test;
		}";
		let predeclared_variables: HashMap<String, String> = HashMap::new();

		let result = parse_declare_instruction(&declare_block, &predeclared_variables, "test", 4);

		if let Ok(_) = result {
			Err("Successfully parsed invalid value")
		} else {
			Ok(())
		}
	}

	#[test]
	fn declare_block_fail_uses_undefined_variable() -> Result<(), &'static str> {
		let declare_block = "{
			test = \"4\";
			mine = \"my \" + thing;
		}";
		let predeclared_variables: HashMap<String, String> = HashMap::new();

		let result = parse_declare_instruction(&declare_block, &predeclared_variables, "test", 4);

		if let Ok(_) = result {
			Err("Successfully parsed undefined variable")
		} else {
			Ok(())
		}
	}

	#[test]
	fn module_block_parses() -> Result<(), String> {
		let module_block = "ModuleName {
			autoload = \"true\";
			log_level = \"debug\";
			extend = log_level + \"_extended\";
		}";
		let predeclared_variables: HashMap<String, String> = HashMap::new();

		let result = parse_module_instruction(&module_block, &predeclared_variables, "test", 5);

		match result {
			Ok(config) => {
				if config.0 == "ModuleName" {
					let mut bad_vars_msgs: Vec<String> = Vec::new();
					format_bad_vars_msg(&mut bad_vars_msgs, &config.1, "autoload", "true");
					format_bad_vars_msg(&mut bad_vars_msgs, &config.1, "log_level", "debug");
					format_bad_vars_msg(&mut bad_vars_msgs, &config.1, "extend", "debug_extended");

					if bad_vars_msgs.is_empty() {
						Ok(())
					} else {
						Err(format!("Variables had incorrect values:\n{}", bad_vars_msgs.join("\n")))
					}
				} else {
					Err(format!(
						"Expected module to be named ModuleName; instead got \"{}\"",
						config.0
					))
				}
			}
			Err(e) => Err(format!("Failed to parse module block declaration: {}", e.message)),
		}
	}

	#[test]
	fn module_block_uses_predeclared_variables() -> Result<(), String> {
		let module_block = "ModuleName {
			autoload = load_this_module;
			log_level = global_log_level;
			extend = log_level + global_log_level_extension;
		}";
		let mut predeclared_variables: HashMap<String, String> = HashMap::new();
		predeclared_variables.insert(String::from("load_this_module"), String::from("true"));
		predeclared_variables.insert(String::from("global_log_level"), String::from("debug"));
		predeclared_variables.insert(String::from("global_log_level_extension"), String::from("_extended"));

		let result = parse_module_instruction(&module_block, &predeclared_variables, "test", 5);

		match result {
			Ok(config) => {
				if config.0 == "ModuleName" {
					let mut bad_vars_msgs: Vec<String> = Vec::new();
					format_bad_vars_msg(&mut bad_vars_msgs, &config.1, "autoload", "true");
					format_bad_vars_msg(&mut bad_vars_msgs, &config.1, "log_level", "debug");
					format_bad_vars_msg(&mut bad_vars_msgs, &config.1, "extend", "debug_extended");

					if bad_vars_msgs.is_empty() {
						Ok(())
					} else {
						Err(format!("Variables had incorrect values:\n{}", bad_vars_msgs.join("\n")))
					}
				} else {
					Err(format!(
						"Expected module to be named ModuleName; instead got \"{}\"",
						config.0
					))
				}
			}
			Err(e) => Err(format!("Failed to parse module block declaration: {}", e.message)),
		}
	}

	#[test]
	fn module_block_fail_no_module_name() -> Result<(), &'static str> {
		let module_block = "{
			autoload = \"false\";
		}";
		let predeclared_variables: HashMap<String, String> = HashMap::new();

		let result = parse_module_instruction(&module_block, &predeclared_variables, "test", 3);

		if let Ok(_) = result {
			Err("Successfully parsed a module block with no module name")
		} else {
			Ok(())
		}
	}

	#[test]
	fn module_block_fail_parse_variables() -> Result<(), &'static str> {
		let module_block = "AModule {
			name = \"module name\";
			bad = \"this line is \" broken;
		}";
		let predeclared_variables: HashMap<String, String> = HashMap::new();

		let result = parse_module_instruction(&module_block, &predeclared_variables, "test", 4);

		if let Ok(_) = result {
			Err("Successfully parsed a module block with syntax errors")
		} else {
			Ok(())
		}
	}

	#[test]
	fn module_block_fail_uses_undefined_variables() -> Result<(), &'static str> {
		let module_block = "ModuleName {
			autoload = \"false\";
			meme = \"use \" + reference;
		}";
		let predeclared_variables: HashMap<String, String> = HashMap::new();

		let result = parse_module_instruction(&module_block, &predeclared_variables, "test", 4);

		if let Ok(_) = result {
			Err("Successfully parsed a module block with undefined variable usage")
		} else {
			Ok(())
		}
	}
}
