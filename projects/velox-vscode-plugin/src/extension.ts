import * as vscode from "vscode";
import { execFile } from "child_process";
import * as diagnostic from "./diagnostics";
import * as utils from "./utils";
import { runBuild } from "./build"; 
import { runCheck, scheduleCheck } from "./check"; 
import { run } from "./run"; 
import { veloxState } from "./state";


export function activate(
    context: vscode.ExtensionContext
) {
    utils.findVeloxConfig();

    if (!veloxState.config) {
        vscode.window.showErrorMessage(
            "No velox.toml found"
        );
        return;
    }


    context.subscriptions.push(
        diagnostic.diagnostics
    );

    context.subscriptions.push(
        vscode.commands.registerCommand(
            "velox.check",
            runCheck
        )
    );

    context.subscriptions.push(
        vscode.commands.registerCommand(
            "velox.build",
            runBuild
        )
    );

    context.subscriptions.push(
        vscode.commands.registerCommand(
            "velox.run",
            run
        )
    );



    context.subscriptions.push(
        vscode.workspace.onDidChangeTextDocument(
            () => scheduleCheck(500)
        )
    );


    context.subscriptions.push(
        vscode.workspace.onDidSaveTextDocument(
            () => scheduleCheck(0)
        )
    );



    scheduleCheck(0);
}



export function deactivate() {}




