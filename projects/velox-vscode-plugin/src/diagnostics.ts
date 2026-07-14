import path = require("path");
import * as vscode from "vscode";

export var diagnostics =
    vscode.languages.createDiagnosticCollection("velox");

export async function parseDiagnostics(
    output: string
) {

    const begin =
        "@@VELOX_EXORDIUM_DIAGNOSTICORUM@@";

    const end =
        "@@VELOX_CLAUSULA_DIAGNOSTICORUM@@";


    const start =
        output.indexOf(begin);

    const finish =
        output.indexOf(end);



    /*
     * Le compilateur n'a pas produit
     * un bloc diagnostic exploitable.
     *
     * On conserve l'état actuel.
     */
    if (
        start === -1 ||
        finish === -1 ||
        finish < start
    ) {

        console.warn(
            "Velox: no diagnostic block found"
        );

        diagnostics.clear();

        return;
    }



    const json =
        output
            .substring(
                start + begin.length,
                finish
            )
            .trim();



    let errors: any[];


    try {

        errors =
            JSON.parse(json);

    }
    catch {

        console.error(
            "Velox: invalid diagnostic JSON:\n", json
        );

        return;
    }



    /*
     * Aucun diagnostic :
     * le code est propre.
     * On supprime les anciens messages.
     */
    if (
        !Array.isArray(errors) ||
        errors.length === 0
    ) {

        diagnostics.clear();

        return;
    }



    const grouped =
        new Map<
            string,
            vscode.Diagnostic[]
        >();



    for (const error of errors) {


        if (
            !error.file ||
            !error.message
        ) {
            continue;
        }



        const uri =
            vscode.Uri.file(
                path.resolve(
                    error.file
                )
            );



        const document =
            vscode.workspace.textDocuments.find(
                d =>
                    d.uri.fsPath === uri.fsPath
            );



        /*
         * Le fichier n'est pas ouvert :
         * on ignore car on ne peut pas
         * convertir correctement les positions.
         */
        if (!document) {
            continue;
        }



        const diagnostic =
            new vscode.Diagnostic(
                new vscode.Range(
                    document.positionAt(
                        error.start_pos
                    ),
                    document.positionAt(
                        error.end_pos
                    )
                ),
                formatMessage(error),
                error.type === "warning"
                    ?
                    vscode.DiagnosticSeverity.Warning
                    :
                    vscode.DiagnosticSeverity.Error
            );



        const list =
            grouped.get(
                uri.fsPath
            ) ?? [];



        list.push(
            diagnostic
        );


        grouped.set(
            uri.fsPath,
            list
        );

    }



    /*
     * Mise à jour complète :
     * on remplace l'ancien état
     * par le nouveau.
     */
    diagnostics.clear();



    for (
        const [
            file,
            list
        ]
        of grouped
    ) {

        diagnostics.set(
            vscode.Uri.file(file),
            list
        );

    }

}




function formatMessage(
    error: any
): string {


    let message =
        error.message;



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
