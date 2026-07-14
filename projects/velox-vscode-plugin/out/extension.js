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
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = __importStar(require("vscode"));
const diagnostic = __importStar(require("./diagnostics"));
const utils = __importStar(require("./utils"));
const build_1 = require("./build");
const check_1 = require("./check");
const run_1 = require("./run");
const state_1 = require("./state");
function activate(context) {
    utils.findVeloxConfig();
    if (!state_1.veloxState.config) {
        vscode.window.showErrorMessage("No velox.toml found");
        return;
    }
    context.subscriptions.push(diagnostic.diagnostics);
    context.subscriptions.push(vscode.commands.registerCommand("velox.check", check_1.runCheck));
    context.subscriptions.push(vscode.commands.registerCommand("velox.build", build_1.runBuild));
    context.subscriptions.push(vscode.commands.registerCommand("velox.run", run_1.run));
    context.subscriptions.push(vscode.workspace.onDidChangeTextDocument(() => (0, check_1.scheduleCheck)(500)));
    context.subscriptions.push(vscode.workspace.onDidSaveTextDocument(() => (0, check_1.scheduleCheck)(0)));
    (0, check_1.scheduleCheck)(0);
}
function deactivate() { }
//# sourceMappingURL=extension.js.map