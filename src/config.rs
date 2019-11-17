use std::collections::HashMap;
use std::fs;
use std::io;
use std::io::prelude::*;

pub struct Config {
	modules: HashMap<String, HashMap<String, String>>,
	connections: HashMap<String, HashMap<String, String>>,
}

impl Config {}
/*
Config blocks:

declare {
	var1 = "butts";
	var2 = var1 + " yes";
}

include "butts/poop.conf"

module ExtraFunctionality {
	usewith = "your mother";
}

connection SomeNetwork {
	server = "127.0.0.1";
	port = "6667";
}
*/

pub enum ConfigError {
	FileError(io::Error),
	ParseError(ConfigParseError),
}

pub struct ConfigParseError {
	file_name: String,
	line_number: u32,
	message: String,
}

struct ConfigFileResult {
	modules: HashMap<String, HashMap<String, String>>,
	connections: HashMap<String, HashMap<String, String>>,
	declared_values: HashMap<String, String>,
}

pub fn read_config(file_name: &str) -> Result<Config, ConfigError> {
	let mut file_name = file_name;
	if file_name == "" {
		file_name = "robobo.conf";
	}

	let config_data = read_config_file(&file_name)?;
	Ok(Config {
		modules: config_data.modules,
		connections: config_data.connections,
	})
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

fn read_config_file(file_name: &str) -> Result<ConfigFileResult, ConfigError> {
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
		if current_byte == 59 {
			// ;
			blocks.push((String::from_utf8_lossy(&buffer).to_string(), line_number));
			buffer.clear();
		} else {
			buffer.push(current_byte);
		}
	}

	let mut declare_blocks = Vec::new();
	let mut include_blocks = Vec::new();
	let mut module_blocks = Vec::new();
	let mut connection_blocks = Vec::new();

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

	let mut declared_variables: HashMap<String, String> = HashMap::new();
	for (block, line) in declare_blocks {
		match parse_declare_instruction(&block, &declared_variables, &file_name, line) {
			Ok(mut result) => {
				for (var, value) in result.drain() {
					declared_variables.insert(var, value);
				}
			}
			Err(e) => return Err(ConfigError::ParseError(e)),
		}
	}

	Err(ConfigError::ParseError(ConfigParseError {
		file_name: String::from(file_name),
		line_number: 0,
		message: String::from("Incomplete parser implementation"),
	}))
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

	let declare_block = declare_block[1..declare_block.len() - 1].trim();
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
				if current_char == ' ' || current_char == '\n' {
					if !buffer.is_empty() {
						current_variable = buffer.drain(..).collect();
						expecting = ParseExpectOperation::AssignOperator;
					}
				} else {
					buffer.push(current_char);
				}
			}
			ParseExpectOperation::AssignOperator => {
				if current_char == ' ' || current_char == '\n' {
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
				} else if !in_string_literal && (current_char == ' ' || current_char == '\n') {
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

#[cfg(test)]
mod tests {
	use super::*;

	// TODO write tests
}
