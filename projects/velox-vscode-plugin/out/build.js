"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.runBuild = runBuild;
const vscode = __importStar(require("vscode"));
const child_process_1 = require("child_process");
const state_1 = require("./state");
const diagnostics_1 = require("./diagnostics");
async function runBuild() {
    if (!state_1.veloxState.config) {
        vscode.window.showErrorMessage("No velox.toml found");
        return false;
    }
    let configFile = state_1.veloxState.config.toString();
    return new Promise(resolve => {
        (0, child_process_1.execFile)("velox-compiler", [
            "build",
            configFile,
            "--diagnostic-format",
            "json"
        ], async (error, stdout, stderr) => {
            await (0, diagnostics_1.parseDiagnostics)(stdout + stderr);
            if (error) {
                resolve(false);
                return;
            }
            resolve(true);
        });
    });
}
function parseBuildResult(output) {
    const begin = "@@VELOX_EXORDIUM_RESULTATI@@";
    const end = "@@VELOX_EXORDIUM_RESULTATI@@";
    const start = output.indexOf(begin);
    const finish = output.indexOf(end);
    if (start === -1 ||
        finish === -1) {
        return false;
    }
    const json = output.substring(start + begin.length, finish)
        .trim();
    const result = JSON.parse(json);
    if (!result.success) {
        return false;
    }
    state_1.veloxState.executable =
        result.executable;
    state_1.veloxState.buildMode =
        result.mode;
    return true;
}
//# sourceMappingURL=build.js.map