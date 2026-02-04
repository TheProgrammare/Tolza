#include "Pipeline_Linker.hpp"

// color_CYAN " [" + std::to_string(left) + "\\" + std::to_string(right) + "] " color_RESET;

bool pipeline_start_linker(const PipelineScripts* pipe_scripts) {/*
	// get all exported modules
	// module name -> ScriptInfo*
	std::unordered_map<std::string, ScriptInfo*> exportMap;
	// build export map
	for (auto& script : scriptsInfo) {
		for (auto& expMod : script->exported_mod) {
			exportMap[expMod] = script.get();
		}
	}

	bool error = false;
	/*
	// for each script, check his imports
	for (auto& script : scriptsInfo) {
		for (auto& impMod : script->imported_mod) {
			if (auto it = exportMap.find(impMod); it != exportMap.end()) {
				if (auto exporter = it->second; exporter != script.get()) {
					// merge the exporter module in the current script
					// WARNING : mergeModule can modify exporter->mod, mse copy if necessary
					Linker linker(script->mod);
					auto clonedMod = CloneModule(exporter->mod);
					if (linker.linkInModule(std::move(clonedMod))) {
						errs() << "Error during the linkage of the exported module: " << impMod << "\n";
						error = true;
					}
				}
			}
		}
	}
	

	return !error;*/
	return false;
}