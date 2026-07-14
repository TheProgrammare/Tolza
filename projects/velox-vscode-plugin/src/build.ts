import * as vscode from "vscode";
import { execFile } from "child_process";
import { veloxState } from "./state";
import { parseDiagnostics } from "./diagnostics";


export async function runBuild() {

    if (!veloxState.config) {

        vscode.window.showErrorMessage(
            "No velox.toml found"
        );

        return false;
    }

    let configFile = veloxState.config.toString();


    return new Promise<boolean>(
        resolve => {


            execFile(
                "velox-compiler",
                [
                    "build",
                    configFile,
                    "--diagnostic-format",
                    "json"
                ],
                async (
                    error,
                    stdout,
                    stderr
                ) => {


                    await parseDiagnostics(
                        stdout + stderr
                    );



                    if (error) {

                        resolve(false);

                        return;
                    }



                    resolve(true);

                }
            );


        }
    );

}

function parseBuildResult(
    output: string
): boolean {


    const begin =
        "@@VELOX_EXORDIUM_RESULTATI@@";

    const end =
        "@@VELOX_EXORDIUM_RESULTATI@@";


    const start =
        output.indexOf(begin);

    const finish =
        output.indexOf(end);


    if (
        start === -1 ||
        finish === -1
    ) {
        return false;
    }


    const json =
        output.substring(
            start + begin.length,
            finish
        )
        .trim();


    const result =
        JSON.parse(json);


    if (!result.success) {
        return false;
    }


    veloxState.executable =
        result.executable;


    veloxState.buildMode =
        result.mode;


    return true;
}