use crate::module_module::{spawn_module, Module};
use crate::protocol_module::Protocol;
use crate::socket_module::Socket;
use crate::CURRENT_BOT_MODULE_VERSION;
use std::collections::HashMap;

struct DataManager {
	modules: HashMap<String, Box<dyn Module>>,
	connections: HashMap<String, Box<dyn Protocol>>,
	socket_creators: HashMap<String, Box<dyn Socket>>,
}

impl DataManager {
	pub fn load_module(&mut self, module_name: &str) -> Result<(), String> {
		if self.modules.contains_key(module_name) {
			return Err(format!("The module \"{}\" is already loaded.", module_name));
		}
		let loaded_module = spawn_module(module_name)?;
		if loaded_module.bot_version != CURRENT_BOT_MODULE_VERSION {
			return Err(format!(
				"The module \"{}\" does not work with the correct bot version (module version: {})",
				module_name, loaded_module.bot_version
			));
		}
		let mut module = loaded_module.module;
		module.load();
		self.modules.insert(String::from(module_name), module);
		Ok(())
	}

	pub fn get_module(&self, module_name: &str) -> Option<&Box<dyn Module>> {
		self.modules.get(module_name)
	}

	pub fn get_module_mut(&mut self, module_name: &str) -> Option<&mut Box<dyn Module>> {
		self.modules.get_mut(module_name)
	}

	pub fn unload_module(&mut self, module_name: &str) -> Result<(), String> {
		let module = self.modules.remove(module_name);
		let mut module = if let Some(module) = module {
			module
		} else {
			return Err(format!("The module \"{}\" wasn't loaded.", module_name));
		};
		module.unload();
		Ok(())
	}
}
