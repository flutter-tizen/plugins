# Flutter-tizen Plugins Style Guide

## Introduction

This style guide outlines the coding conventions for contributions to the
flutter-tizen/plugins repository.

## Style Guides

Code should follow the relevant style guides, and use the correct
auto-formatter, for each language, as described in
[the repository contributing guide's Style section](https://github.com/flutter/packages/blob/main/CONTRIBUTING.md#style).

## Best Practices

- Code should follow the guidance and principles described in
  [the flutter/packages contribution guide](https://github.com/flutter/flutter/blob/master/docs/ecosystem/contributing/README.md).
- For Flutter code, the
  [Flutter styleguide](https://github.com/flutter/flutter/blob/main/docs/contributing/Style-guide-for-Flutter-repo.md)
  should be followed as the first priority, and
  [Effective Dart: Style](https://dart.dev/effective-dart/style)
  should only be followed when it does not conflict with the former.
- Code should be tested. Changes to plugin packages, which include code written
  in C, C++, Java, Kotlin, Objective-C, or Swift, should have appropriate tests
  as described in [the plugin test guidance](https://github.com/flutter/flutter/blob/master/docs/ecosystem/testing/Plugin-Tests.md).

## Miscellaneous Languages

- C++ code is formatted using `clang-format`, linted with `clang-tidy`, and should follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

## Review Agent Guidelines

- Only review changes to the `master` branch. Other changes have already been reviewed (and are being cherrypicked).
  
When providing a summary, the review agent must adhere to the following principles:
- Only review changes to the `master` branch. Other changes have already been reviewed (and are being cherrypicked).
- **Be Objective:** Focus on a neutral, descriptive summary of the changes. Avoid subjective value judgments
  like "good," "bad," "positive," or "negative." The goal is to report what the code does, not to evaluate it.
- **Use Code as the Source of Truth:** Base all summaries on the code diff. Do not trust or rephrase the PR
  description, which may be outdated or inaccurate. A summary must reflect the actual changes in the code.
- **Be Concise:** Generate summaries that are brief and to the point. Focus on the most significant changes,
  and avoid unnecessary details or verbose explanations. This ensures the feedback is easy to scan and understand.

### Temporary dependency_overrides

It is normal practice for PRs that affect multiple packages in a federated plugin to contain a temporary
`dependency_overrides` section during initial review. The review agent should not comment about
`dependency_overrides` sections in `pubspec.yaml` files that have a comment starting with:
"FOR TESTING AND INITIAL REVIEW ONLY. DO NOT MERGE."

## Documentation

- All public members should have documentation.
- **Answer your own questions**: If you have a question, find the answer, and then document it where you first looked.
- **Documentation should be useful**: Explain the *why* and the *how*.
- **Introduce terms**: Assume the reader does not know everything. Link to definitions.
- **Provide sample code**: Use `{@tool dartpad}` for runnable examples.
  - Inline code samples are contained within `{@tool dartpad}` and `{@end-tool}`, and use the format of the following example to insert the 
- **Provide illustrations or screenshots** for widgets.
- Use `///` for public-quality documentation, even on private members.

## Further Reading

For more detailed guidance, refer to the following documents:

- [Style guide for the Flutter repository](https://github.com/flutter/flutter/blob/main/docs/contributing/Style-guide-for-Flutter-repo.md)
- [Effective Dart: Style](https://dart.dev/effective-dart/style)
- [Tree Hygiene](https://github.com/flutter/flutter/blob/main/docs/contributing/Tree-hygiene.md)
- [The Flutter contribution guide](https://github.com/flutter/flutter/blob/main/CONTRIBUTING.md)
- [Writing effective tests guide](https://github.com/flutter/flutter/blob/main/docs/contributing/testing/Writing-Effective-Tests.md)
- [Running and writing tests guide](https://github.com/flutter/flutter/blob/main/docs/contributing/testing/Running-and-writing-tests.md)
- [Engine testing guide](https://github.com/flutter/flutter/blob/main/docs/engine/testing/Testing-the-engine.md)
