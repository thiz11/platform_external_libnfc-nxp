/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * File            : com_trustedlogic_trustednfc_android_NdefRecord.c
 * Original-Author : Trusted Logic S.A. (Jeremie Corbier)
 * Created         : 08-10-2009
 */

#include "trustednfc_jni.h"

namespace android {

static jbyteArray com_trustedlogic_trustednfc_android_NdefRecord_generate(
   JNIEnv *e, jobject o, jshort flags, jshort tnf, jbyteArray type,
   jbyteArray id, jbyteArray payload)
{
   NFCSTATUS status;
   phFriNfc_NdefRecord_t record;
   uint32_t buf_size;
   uint32_t record_size;
   uint8_t *buf = NULL;
   jbyteArray result = NULL;

   /* Prepare NDEF record structure */
   record.Flags = (uint8_t)flags;
   record.Tnf = (uint8_t)tnf;
   record.TypeLength = (uint32_t)e->GetArrayLength(type);
   record.Type = (uint8_t *)e->GetByteArrayElements(type, NULL);
   record.IdLength = (uint32_t)e->GetArrayLength(id);
   record.Id = (uint8_t *)e->GetByteArrayElements(id, NULL);
   record.PayloadLength = (uint32_t)e->GetArrayLength(payload);
   record.PayloadData = (uint8_t *)e->GetByteArrayElements(payload, NULL);

   buf_size = record.PayloadLength + record.IdLength + record.TypeLength + 8;

   buf = (uint8_t*)malloc(buf_size);
   if(buf == NULL)
      goto end;

   LOGD("phFriNfc_NdefRecord_Generate()");
   REENTRANCE_LOCK();
   status = phFriNfc_NdefRecord_Generate(&record, buf, buf_size, &record_size);
   REENTRANCE_UNLOCK();
   if(status != NFCSTATUS_SUCCESS)
   {
      LOGE("phFriNfc_NdefRecord_Generate() returned 0x%04x[%s]", status, trustednfc_jni_get_status_name(status));
      goto end;   
   }
   LOGD("phFriNfc_NdefRecord_Generate() returned 0x%04x[%s]", status, trustednfc_jni_get_status_name(status));

   result = e->NewByteArray(record_size);
   if(result == NULL)
      goto end;

   e->SetByteArrayRegion(result, 0, record_size, (jbyte *)buf);

end:
   e->ReleaseByteArrayElements(type, (jbyte *)record.Type, JNI_ABORT);
   e->ReleaseByteArrayElements(id, (jbyte *)record.Id, JNI_ABORT);
   e->ReleaseByteArrayElements(payload, (jbyte *)record.PayloadData, JNI_ABORT);

   if(buf)
      free(buf);

   return result;
}

/*
 * JNI registration.
 */
static JNINativeMethod gMethods[] =
{
   {"generate", "(SS[B[B[B)[B",
      (void *)com_trustedlogic_trustednfc_android_NdefRecord_generate},
};

int register_com_trustedlogic_trustednfc_android_NdefRecord(JNIEnv *e)
{
   return jniRegisterNativeMethods(e,
      "com/trustedlogic/trustednfc/android/NdefRecord",
      gMethods, NELEM(gMethods));
}

} // namespace android
