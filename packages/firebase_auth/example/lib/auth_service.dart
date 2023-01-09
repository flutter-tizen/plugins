// ignore_for_file: public_member_api_docs

import 'dart:convert';
import 'dart:math';

import 'package:crypto/crypto.dart';
import 'package:desktop_webview_auth/desktop_webview_auth.dart';
import 'package:desktop_webview_auth/google.dart';
import 'package:desktop_webview_auth/twitter.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:flutter/material.dart';
import 'package:sign_in_with_apple/sign_in_with_apple.dart';
import 'package:webview_auth_tizen/providers/google.dart';
import 'package:webview_auth_tizen/providers/facebook.dart';
import 'package:webview_auth_tizen/providers/github.dart';

import 'auth.dart';

const _redirectUri =
    'https://react-native-firebase-testing.firebaseapp.com/__/auth/handler';
const _googleClientId =
    '448618578101-sg12d2qin42cpr00f8b0gehs5s7inm0v.apps.googleusercontent.com';
const _twitterApiKey = '';
const _twitterApiSecretKey = '';
const _facebookClientId = '';
const _githubClientId = '';
const _githubClientSecret = '';

/// Provide authentication services with [FirebaseAuth].
class AuthService {
  final _auth = FirebaseAuth.instance;

  Future<void> emailAuth(
    AuthMode mode, {
    required String email,
    required String password,
  }) {
    assert(mode != AuthMode.phone);

    try {
      if (mode == AuthMode.login) {
        return _auth.signInWithEmailAndPassword(
          email: email,
          password: password,
        );
      } else {
        return _auth.createUserWithEmailAndPassword(
          email: email,
          password: password,
        );
      }
    } catch (e) {
      rethrow;
    }
  }

  Future<void> anonymousAuth() {
    try {
      return _auth.signInAnonymously();
    } catch (e) {
      rethrow;
    }
  }

  Future<void> phoneAuth({
    required String phoneNumber,
    required Future<String?> Function() smsCode,
  }) async {
    try {
      final confirmationResult =
          await FirebaseAuth.instance.signInWithPhoneNumber(phoneNumber);

      final code = await smsCode.call();

      if (code != null) {
        await confirmationResult.confirm(code);
      } else {
        return;
      }
    } catch (e) {
      rethrow;
    }
  }

  Future<void> googleSignIn(BuildContext context) async {
    try {
      // Handle login by a third-party provider.
      final result = await GoogleLoginPage.signIn(
        _googleClientId,
        'https://www.googleapis.com/auth/userinfo.email',
        _redirectUri,
        context,
      );

      // Create a new credential
      final credential = GoogleAuthProvider.credential(
        idToken: result.idToken,
        accessToken: result.accessToken,
      );

      // Once signed in, return the UserCredential
      await _auth.signInWithCredential(credential);
    } on FirebaseAuthException catch (_) {
      rethrow;
    }
  }

  Future<void> twitterSignIn() async {
    try {
      // Handle login by a third-party provider.
      final result = await DesktopWebviewAuth.signIn(
        TwitterSignInArgs(
          apiKey: _twitterApiKey,
          apiSecretKey: _twitterApiSecretKey,
          redirectUri: _redirectUri,
        ),
      );

      if (result != null) {
        // Create a new credential
        final credential = TwitterAuthProvider.credential(
          secret: result.tokenSecret!,
          accessToken: result.accessToken!,
        );

        // Once signed in, return the UserCredential
        await _auth.signInWithCredential(credential);
      } else {
        return;
      }
    } on FirebaseAuthException catch (_) {
      rethrow;
    }
  }

  Future<void> facebookSignIn(BuildContext context) async {
    try {
      // Handle login by a third-party provider.
      final result = await FacebookLoginPage.signIn(
        _facebookClientId,
        _redirectUri,
        context,
      );

      final credential = FacebookAuthProvider.credential(result.accessToken!);

      // Once signed in, return the UserCredential
      await _auth.signInWithCredential(credential);
    } on FirebaseAuthException catch (_) {
      rethrow;
    }
  }

  Future<void> githubSignIn(BuildContext context) async {
    try {
      // Handle login by a third-party provider.
      final result = await GithubLoginPage.signIn(
        _githubClientId,
        'user',
        _redirectUri,
        _githubClientSecret,
        context,
      );

      // Create a new credential
      final credential = GithubAuthProvider.credential(result.accessToken!);

      // Once signed in, return the UserCredential
      await _auth.signInWithCredential(credential);
    } on FirebaseAuthException catch (_) {
      rethrow;
    }
  }

  /// Generates a cryptographically secure random nonce, to be included in a
  /// credential request.
  String _generateNonce([int length = 32]) {
    const charset =
        '0123456789ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvwxyz-._';
    final random = Random.secure();
    return List.generate(length, (_) => charset[random.nextInt(charset.length)])
        .join();
  }

  /// Returns the sha256 hash of [input] in hex notation.
  String _sha256ofString(String input) {
    final bytes = utf8.encode(input);
    final digest = sha256.convert(bytes);
    return digest.toString();
  }

  Future appleSignIn() async {
    try {
      final rawNonce = _generateNonce();
      final nonce = _sha256ofString(rawNonce);

      final credential = await SignInWithApple.getAppleIDCredential(
        scopes: [
          AppleIDAuthorizationScopes.email,
          AppleIDAuthorizationScopes.fullName,
        ],
      );

      debugPrint('${credential.state}');

      if (credential.identityToken != null) {
        // Create an `OAuthCredential` from the credential returned by Apple.
        final oauthCredential = OAuthProvider('apple.com').credential(
          idToken: credential.identityToken,
          rawNonce: nonce,
        );

        // Sign in the user with Firebase. If the nonce we generated earlier does
        // not match the nonce in `appleCredential.identityToken`, sign in will fail.
        await FirebaseAuth.instance.signInWithCredential(oauthCredential);
      } else {
        return;
      }
    } on FirebaseAuthException catch (_) {
      rethrow;
    }
  }

  Future<void> resetPassword(String email) {
    try {
      return FirebaseAuth.instance.sendPasswordResetEmail(email: email);
    } catch (e) {
      rethrow;
    }
  }

  Future<void> linkWithGoogle() async {
    try {
      final result = await DesktopWebviewAuth.signIn(
        GoogleSignInArgs(
          clientId: _googleClientId,
          redirectUri: _redirectUri,
          scope: 'https://www.googleapis.com/auth/userinfo.email',
        ),
      );

      if (result != null) {
        // Create a new credential
        final credential = GoogleAuthProvider.credential(
          accessToken: result.accessToken,
          idToken: result.idToken,
        );

        // Once signed in, return the UserCredential
        await _auth.currentUser?.linkWithCredential(credential);
      }
    } on FirebaseAuthException catch (_) {
      rethrow;
    }
  }

  /// Sign the Firebase user out.
  Future<void> signOut() async {
    await _auth.signOut();
  }
}
