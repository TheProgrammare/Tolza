import * as vscode from "vscode";
import * as fs from "fs"
import path = require("path");
import { veloxState } from "./state";

export function findVeloxConfig() {

    const workspace =
        vscode.workspace.workspaceFolders?.[0];

    if (!workspace) {
        return undefined;
    }


    let current =
        workspace.uri.fsPath;


    while (true) {

        const config =
            path.join(
                current,
                "velox.toml"
            );


        if (fs.existsSync(config)) {
            veloxState.config = config;
            return;
        }


        const parent =
            path.dirname(current);


        if (parent === current) {
            veloxState.config = undefined
            return;
        }


        current = parent;
    }
}
