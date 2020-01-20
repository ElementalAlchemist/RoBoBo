use crate::config;
use crate::logger;
use std::sync::{Arc, Mutex};

pub trait Protocol {} // TODO

pub struct ProtocolData {
	pub name: String,
	pub connect_addr: String,
	pub connect_port: u16,
	pub logger: Arc<Mutex<logger::Logger>>,
}

impl ProtocolData {
	pub fn new(
		name: &str,
		config: &config::ConnectionData,
		logger: Arc<Mutex<logger::Logger>>,
	) -> Result<ProtocolData, String> {
		let config_data = config.get_data();
		let connect_addr = if config_data.contains_key("server") {
			config_data["server"].clone()
		} else {
			return Err(format!(
				"Can't connect to {}: configuration doesn't contain a server",
				name
			));
		};

		let connect_port = if config_data.contains_key("port") {
			config_data["port"].clone()
		} else {
			return Err(format!(
				"Can't connect to {}: configuration doesn't contain a port",
				name
			));
		};

		let connect_port: u16 = match connect_port.parse() {
			Ok(port) => port,
			Err(err) => {
				return Err(format!(
					"Can't connect to {}: invalid port, couldn't parse as port number ({})",
					name, err
				))
			}
		};

		Ok(ProtocolData {
			name: String::from(name),
			connect_addr,
			connect_port,
			logger,
		})
	}
}
