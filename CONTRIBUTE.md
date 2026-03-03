<div align=center>
  
  ![🇫🇷 Français](docs/CONTRIBUTE_FR.md)

  # CONTRIBUTE

</div>

# How to contribute ?
Before consedering to contribute to this project, please read some documentations:
- ![Contribute](CONTRIBUTE.md) (this document)
- ![Project Presentation](README.md) (for all)
- ![Technical Implementation](MANIFEST.md) (for paradigms, concepts, syntax, behaviour)
- The source code preliminarily (for code contributors)

## Contribute to the code
1. Clone this repository to your machine
2. Follow ![Contribute Code](docs/CONTRIBUTE_CODE.md) software installation
3. Contribute to the code by following ![Pull Request Rules](docs/CONTRIBUTE_CODE.md/Pull_Request_Rules)
4. Write a ![pull request template](docs/PULL_REQUEST_TEMPLATE.md) to explain the pull request
>  If you make an innovation, add a ![RFC template](docs/RFC_TEMPLATE.md) to explain and justify the innovation
5. Make your pull resquest

## Contribute to the philosophy
1. Makes sure that your suggestion is not duplicated or already rejected
2. Write a ![Philosophical template](docs/PHILOSOPHICAL_TEMPLATE.md) to explain your suggestion
3. Don't hesitate to integrate graphical representation, examples, pseudo code, ...
4. Exchange ideas and discuss with the community to refine your proposal, or even turn it into a community proposal.

# Who can contribute ?
Any volunteer, but here are a few ideas on how you can make yourself useful based on your skills

| my skill | my utility for velox |
|-|-|
| junior programmer | better innocent understanding of overly complex concepts, better simplified approach, closer to spontaneous understanding |
| senior programmer | good for the code review, experience about paradigm and concepts, better user experience approach |
| non programmer | good for educative/novice approach, for explicit documentation |
| linguist | pertinent for the code syntax, the human readable approach |
| manager | good view for scalable projects, pertinent workflow and workspace | 

# Project Objectives
**Résumé**

Ce projet a pour objectif de répondre à des besoins raisonnables sur des cas concrets et pratiques, à la fois via le langage Velox et sa toolchain.  

### Langage (Velox)
- Langage de programmation **déterministe, prévisible et explicite**, avec une **syntaxe simple et moderne**.
- Les **paradigmes natifs** du langage doivent être utiles sans introduire une complexité excessive.
- L’**abstraction** est appréciée, mais le programmeur doit pouvoir comprendre globalement ce qui se passe. L’abstraction passe donc par la **simplicité et la clarté du code**.

### Sectorisation des fonctionnalités
- Les fonctionnalités **très pratiques et répandues** doivent être considérées comme **natives au langage**.  
  Exemple : les flottants fixes (`deci`, `udeci`) pour un domaine spécialisé peuvent être intégrés directement.
- Les fonctionnalités **pratiques et répandues mais complexes**, nécessitant des scripts intermédiaires, doivent être intégrées dans la **stdlib** et **accessibles comme si elles étaient natives**.
- Les fonctionnalités **répandues et optimisées**, utiles dans des domaines généraux, doivent être mises dans la **stdlib**.
- Les fonctionnalités **spécialisées, lourdes ou non essentielles** doivent être mises dans des **packages séparés**.

### Toolchain
- Fournir un langage **clé en main**, adapté autant aux **petits projets** qu’aux **projets industriels**.
- La toolchain est à utiliser de préférence dans un **espace de travail**.
- `velox.config` est le **fichier de référence**, indiquant le comportement du compilateur, les dossiers cibles, et définissant l’espace de travail par la localisation des dossiers.
- La toolchain n’est pas basique mais doit demeurer **simple, pratique et intuitive**.


# Governance
- **Founder**:  
  The founder has the final say on all decisions regarding the language, compiler, and toolchain, as well as design, concept, syntax, and pull requests. This ensures a **strong overall coherence** and clear project direction, avoiding excessive complexity as seen in some committees.

- **Community**:  
  The community can propose improvements or changes to any part of the project. Proposals are regularly reviewed by the most active or prominent members. Interesting proposals, even if less visible, may be put on the agenda if they appear sufficiently relevant.

- **Purpose of this governance**:  
  Prevent fragmentation in the development of the language, compiler, and toolchain, while maintaining **overall coherence**, simplicity, and alignment with the project’s objectives.
