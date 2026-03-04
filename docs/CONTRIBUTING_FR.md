# Comment contribuer ?
Avant de considérer de contribuer à ce projet, veuillez lire quelques documentations:
- ![Contributing](CONTRIBUTING_FR.md) (ce document)
- ![Project Presentation](docs/README_FR.md) (pour tous)
- ![Technical Implementation](MANIFEST.md) (pour les paradgimes, concepts, syntax, comportement)
- Le code source de manière liminaire (pour les contributeurs de code)

## Contribuer au code
1. Cloner ce répertoire sur votre machine
2. Suivez les instruction de ![Contributing Code](docs/CONTRIBUTING_CODE.md) pour l'intallation logiciel
3. Contribuez au code en respectant ![Pull Request Rules](docs/CONTRIBUTING_CODE.md#Pull-Request-Rules)
4. Rédigez (en anglais ou français) ![pull request template](docs/PULL_REQUEST_TEMPLATE.md) pour expliquer le pull request
> Si vous innovez, ajoutez ![RFC template](docs/RFC_TEMPLATE.md) pour expliquer et justifier l'innovation
5. Réalisez votre pull request 

## Contribuer à la philosophie
1. Assurez-vous que votre suggestion n'est pas dupliquée ou déjà rejettée
2. Rédigez (en anglais ou français) ![Philosophical template](docs/PHILOSOPHICAL_TEMPLATE.md) pour expliquer votre suggestion
3. N'hésitez pas à intégrer des représentations graphiques, exemples, pseudo code, ...
4. Échanger les idées et débattez avec la communauté pour parfaire votre proposition, ou même le transformer en suggestion de communauté.

# Qui peut contribuer ?
N'importe quel volontaire, mais il y a quelques suggestions sur comment vous pouvez vous rendre utile selon vos compétences

| mes compétences | mon utilité pour velox |
|-|-|
| jeunes développeur | une meilleur compréhension innocente sur les conceptes trop complexe, une meilleure approche simplifiée, proche d'une compréhension spontanée |
| développeur expérimentés | bon pour la revue de code, expérience sur les paradigmes et concepts, meilleur expérence dans les apporches |
| non développeur | bon pour l'éducation et approche novice, pour la documentation explicite |
| linguiste | pertinent pour la syntaxe de code, apporche de documentation explicite |
| manager | bonne vue de projets échelonnable, espace de travaille et flux de travail pertinent | 

# Objectifs du Projet
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

# Structure du projet
Ce projet contient quelques sous-projets:
- Compiler
  - Lexer
  - Parser
  - FFI Binder (and FFI JSON Reader)
  - Resolver
  - Codegen
- Toolchain
  - Commands
  - Package Manager
- Visual studio code plugin

# Gouvernance
- **Fondateur** :  
  Le fondateur a le dernier mot sur toutes les décisions liées au langage, au compilateur, à la toolchain, ainsi qu’au design, au concept, à la syntaxe et aux pull requests. Cela garantit une **cohérence globale solide** et la direction claire du projet, évitant les complexifications excessives observées dans certains comités.

- **Communauté** :  
  La communauté peut proposer des améliorations ou des modifications sur n’importe quel aspect du projet. Ces propositions sont auditées régulièrement par les membres les plus actifs ou mis en avant. Des propositions intéressantes, même moins visibles, peuvent être mises à l’ordre du jour si elles semblent suffisamment pertinentes.

- **Objectif de cette gouvernance** :  
  Éviter la dispersion dans le développement du langage, du compilateur et de la toolchain, tout en maintenant la **cohérence globale**, la simplicité et l’alignement avec les objectifs du projet.

