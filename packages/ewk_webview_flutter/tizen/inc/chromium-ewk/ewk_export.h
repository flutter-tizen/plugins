// Copyright 2013 Samsung Electronics. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EWK_EXPORT_H_
#define EWK_EXPORT_H_

#ifdef EXPORT_API
#undef EXPORT_API
#endif

#ifdef __GNUC__
#if __GNUC__ >= 4
#define EXPORT_API __attribute__((visibility("default")))
#else
#define EXPORT_API
#endif
#else
#define EXPORT_API
#endif

#endif  // EWK_EXPORT_H
