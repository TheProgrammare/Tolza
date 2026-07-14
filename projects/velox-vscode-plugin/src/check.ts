import * as vscode from "vscode";
import { veloxState } from "./state";
import { execFile } from "child_process";
import { parseDiagnostics } from "./diagnostics";

let checkTimer: NodeJS.Timeout | undefined;

let checkGeneration = 0;

let checkRunning = false;
let checkPending = false;

export function scheduleCheck(
    delay: number
) {

    if (checkTimer) {
        clearTimeout(checkTimer);
    }


    checkTimer =
        setTimeout(
            () => {

                runCheck();

            },
            delay
        );
}

export async function runCheck() {

    if (!veloxState.config) {
        return;
    }


    if (checkRunning) {

        checkPending = true;

        return;
    }


    checkRunning = true;


    /*
     * Synchronise le disque avec le buffer VS Code
     */
    await vscode.workspace.saveAll();



    const generation =
        ++checkGeneration;



    execFile(
        "velox-compiler",
        [
            "check",
            veloxState.config,
            "--diagnostic-format",
            "json"
        ],
        async (
            _error,
            stdout,
            stderr
        ) => {


            if (
                generation !== checkGeneration
            ) {

                checkRunning = false;

                return;
            }



            await parseDiagnostics(
                stdout + stderr
            );



            checkRunning = false;



            if (checkPending) {

                checkPending = false;

                runCheck();

            }

        }
    );
}