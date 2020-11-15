use libloading::{Library, Symbol};

pub trait Module {
	fn load(&mut self);
	fn unload(&mut self);

	// TODO IRC events
}

pub struct LoadedModule {
	pub module: Box<dyn Module>,
	pub bot_version: u32,
}

pub fn spawn_module(name: &str) -> Result<LoadedModule, String> {
	let load_path = format!("modules/lib{}.so", name);
	let module_lib = match Library::new(load_path) {
		Ok(result) => result,
		Err(err) => return Err(format!("Failed to load module {}: {}", name, err)),
	};

	let module = unsafe {
		let load_func: Symbol<fn() -> Box<dyn Module>> = match module_lib.get(b"spawn") {
			Ok(func) => func,
			Err(err) => return Err(format!("Module \"{}\" doesn't contain a spawn function: {}", name, err)),
		};
		load_func()
	};
	let bot_version = unsafe {
		let version_func: Symbol<fn() -> u32> = match module_lib.get(b"bot_version") {
			Ok(func) => func,
			Err(err) => {
				return Err(format!(
					"Module \"{}\" doesn't contain a bot version function: {}",
					name, err
				))
			}
		};
		version_func()
	};
	Ok(LoadedModule { module, bot_version })
}
