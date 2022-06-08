/*
 * Copyright (C) 2018 Samsung Electronics. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY SAMSUNG ELECTRONICS. AND ITS CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMSUNG ELECTRONICS. OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    ewk_media_parental_rating_info.h
 * @brief   This file describes the ewk media parental rating info API.
 */

#ifndef ewk_media_parental_rating_info_h
#define ewk_media_parental_rating_info_h

#include "ewk_media_parental_rating_info_product.h"
#include "private/ewk_private.h"

#ifdef __cplusplus
extern "C" {
#endif

Ewk_Media_Parental_Rating_Info* ewkMediaParentalRatingInfoCreate(
    const char* info, const char* url) {
  Ewk_Media_Parental_Rating_Info* data = new Ewk_Media_Parental_Rating_Info;
  data->parentalInfoStr = eina_stringshare_add(info ? info : "");
  data->url = eina_stringshare_add(url ? url : "");
  return data;
}

void ewkMediaParentalRatingInfoDelete(Ewk_Media_Parental_Rating_Info* data) {
  if (data->parentalInfoStr) eina_stringshare_del(data->parentalInfoStr);
  if (data->url) eina_stringshare_del(data->url);
  delete data;
}

#ifdef __cplusplus
}
#endif
#endif  // ewk_media_parental_rating_info_h
