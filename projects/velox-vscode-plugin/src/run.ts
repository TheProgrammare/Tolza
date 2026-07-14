import * as vscode from "vscode";
import { execFile } from "child_process";
import { veloxState } from "./state";
import { runBuild } from "./build";


export async function run() {
    const success =
        await runBuild();



    if (!success) {

        vscode.window.showErrorMessage(
            "Build failed"
        );

        return;
    }



    if (!veloxState.executable) {

        vscode.window.showErrorMessage(
            "Executable not found"
        );

        return;
    }



    const terminal =
        vscode.window.createTerminal(
            "Velox Run"
        );


    terminal.show();


    terminal.sendText(
        veloxState.executable
    );

}