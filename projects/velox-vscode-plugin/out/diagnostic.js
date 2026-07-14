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
exports.diagnostics = void 0;
exports.parseDiagnostics = parseDiagnostics;
const path = require("path");
const vscode = __importStar(require("vscode"));
exports.diagnostics = vscode.languages.createDiagnosticCollection("velox");
async function parseDiagnostics(output) {
    const begin = "@@VELOX_EXORDIUM_DIAGNOSTICORUM@@";
    const end = "@@VELOX_CLAUSULA_DIAGNOSTICORUM@@";
    const start = output.indexOf(begin);
    const finish = output.indexOf(end);
    /*
     * Le compilateur n'a pas produit
     * un bloc diagnostic exploitable.
     *
     * On conserve l'état actuel.
     */
    if (start === -1 ||
        finish === -1 ||
        finish < start) {
        console.warn("Velox: no diagnostic block found");
        exports.diagnostics.clear();
        return;
    }
    const json = output
        .substring(start + begin.length, finish)
        .trim();
    let errors;
    try {
        errors =
            JSON.parse(json);
    }
    catch {
        console.error("Velox: invalid diagnostic JSON:\n", json);
        return;
    }
    /*
     * Aucun diagnostic :
     * le code est propre.
     * On supprime les anciens messages.
     */
    if (!Array.isArray(errors) ||
        errors.length === 0) {
        exports.diagnostics.clear();
        return;
    }
    const grouped = new Map();
    for (const error of errors) {
        if (!error.file ||
            !error.message) {
            continue;
        }
        const uri = vscode.Uri.file(path.resolve(error.file));
        const document = vscode.workspace.textDocuments.find(d => d.uri.fsPath === uri.fsPath);
        /*
         * Le fichier n'est pas ouvert :
         * on ignore car on ne peut pas
         * convertir correctement les positions.
         */
        if (!document) {
            continue;
        }
        const diagnostic = new vscode.Diagnostic(new vscode.Range(document.positionAt(error.start_pos), document.positionAt(error.end_pos)), formatMessage(error), error.type === "warning"
            ?
                vscode.DiagnosticSeverity.Warning
            :
                vscode.DiagnosticSeverity.Error);
        const list = grouped.get(uri.fsPath) ?? [];
        list.push(diagnostic);
        grouped.set(uri.fsPath, list);
    }
    /*
     * Mise à jour complète :
     * on remplace l'ancien état
     * par le nouveau.
     */
    exports.diagnostics.clear();
    for (const [file, list] of grouped) {
        exports.diagnostics.set(vscode.Uri.file(file), list);
    }
}
function formatMessage(error) {
    let message = error.message;
    if (error.code) {
        message =
            `[${error.code}] ${message}`;
    }
    if (error.hint) {
        message +=
            `\n\nHint: ${error.hint}`;
    }
    return message;
}
//# sourceMappingURL=diagnostic.js.map