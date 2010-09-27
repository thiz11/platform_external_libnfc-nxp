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
 * File            : com_trustedlogic_trustednfc_android_NdefMessage.c
 * Original-Author : Trusted Logic S.A. (Jeremie Corbier)
 * Created         : 06-10-2009
 */

#include "trustednfc_jni.h"

namespace android {

static jint com_trustedlogic_trustednfc_android_NdefMessage_parseNdefMessage(
   JNIEnv *e, jobject o, jbyteArray array)
{
   NFCSTATUS status;
   uint32_t i;
   jbyte *raw_msg;
   jsize raw_msg_size;
   uint32_t num_of_records = 0;
   uint8_t **records = NULL;
   uint8_t *is_chunked = NULL;
   jint ret = -1;
   phFriNfc_NdefRecord_t record;

   jclass record_cls;
   jobjectArray records_array;
   jmethodID ctor;

   jclass msg_cls;
   jfieldID mrecords;

   raw_msg_size = e->GetArrayLength(array);
   raw_msg = e->GetByteArrayElements(array, NULL);
   if(raw_msg == NULL)
      return -1;

   /* Get the number of records in the message so we can allocate buffers */
   LOGD("phFriNfc_NdefRecord_GetRecords(NULL)");
   REENTRANCE_LOCK();
   status = phFriNfc_NdefRecord_GetRecords((uint8_t *)raw_msg,
      (uint32_t)raw_msg_size, NULL, NULL, &num_of_records);
   REENTRANCE_UNLOCK();
   if(status != NFCSTATUS_SUCCESS)
   {
      LOGE("phFriNfc_NdefRecord_GetRecords(NULL) returned 0x%04x[%s]", status, trustednfc_jni_get_status_name(status));
      goto end;   
   }
   LOGD("phFriNfc_NdefRecord_GetRecords(NULL) returned 0x%04x[%s]", status, trustednfc_jni_get_status_name(status));

   LOGD("found %d records in message", num_of_records);

   is_chunked = (uint8_t*)malloc(num_of_records);
   if(is_chunked == NULL)
      goto end;
   records = (uint8_t**)malloc(num_of_records * sizeof(uint8_t *));
   if(records == NULL)
      goto end;

   /* Now, actually retrieve records position in message */
   LOGD("phFriNfc_NdefRecord_GetRecords()");
   REENTRANCE_LOCK();
   status = phFriNfc_NdefRecord_GetRecords((uint8_t *)raw_msg,
      (uint32_t)raw_msg_size, records, is_chunked, &num_of_records);
   REENTRANCE_UNLOCK();
   if(status != NFCSTATUS_SUCCESS)
   {
      LOGE("phFriNfc_NdefRecord_GetRecords() returned 0x%04x[%s]", status, trustednfc_jni_get_status_name(status));
      goto end;   
   }
   LOGD("phFriNfc_NdefRecord_GetRecords() returned 0x%04x[%s]", status, trustednfc_jni_get_status_name(status));

   /* Build NDEF records array */
   record_cls =
      e->FindClass("com/trustedlogic/trustednfc/android/NdefRecord");
   records_array = e->NewObjectArray((jsize)num_of_records, record_cls,
      NULL);
   if(records_array == NULL)
      goto end;

   ctor = e->GetMethodID(record_cls, "<init>", "(S[B[B[B)V");
   
   LOGD("NFC_Number of records = %d\n",num_of_records);

   for(i = 0; i < num_of_records; i++)
   {
      jbyteArray type, id, payload;
      jobject new_record;

      LOGD("phFriNfc_NdefRecord_Parse()");
      REENTRANCE_LOCK();
      status = phFriNfc_NdefRecord_Parse(&record, records[i]);
      REENTRANCE_UNLOCK();
      if(status != NFCSTATUS_SUCCESS)
      {
         LOGE("phFriNfc_NdefRecord_Parse() returned 0x%04x[%s]", status, trustednfc_jni_get_status_name(status));
         goto end;   
      }
      LOGD("phFriNfc_NdefRecord_Parse() returned 0x%04x[%s]", status, trustednfc_jni_get_status_name(status));

      type = e->NewByteArray(record.TypeLength);
      if(type == NULL)
      {
         LOGD("NFC_Set Record Type Error\n");
         goto end;
      }

      id = e->NewByteArray(record.IdLength);
      if(id == NULL)
      {
         LOGD("NFC_Set Record ID Error\n");
         goto end;
      }

      payload = e->NewByteArray(record.PayloadLength);
      if(payload == NULL)
      {
         LOGD("NFC_Set Record Payload Error\n");
         goto end;
      }

      e->SetByteArrayRegion(type, 0, record.TypeLength,
         (jbyte *)record.Type);
      e->SetByteArrayRegion(id, 0, record.IdLength,
         (jbyte *)record.Id);
      e->SetByteArrayRegion(payload, 0, record.PayloadLength,
         (jbyte *)record.PayloadData);


     new_record = e->NewObject(record_cls, ctor,
         (jshort)record.Tnf, type, id, payload);        


      e->SetObjectArrayElement(records_array, i, new_record);

      /* Try not to clutter the Java stack too much */
      e->DeleteLocalRef(new_record);
      e->DeleteLocalRef(type);
      e->DeleteLocalRef(id);
      e->DeleteLocalRef(payload);
   }

   /* Store built array in our NDEFMessage instance */
   msg_cls = e->GetObjectClass(o);
   mrecords = e->GetFieldID(msg_cls, "mRecords",
      "[Lcom/trustedlogic/trustednfc/android/NdefRecord;");

   e->SetObjectField(o, mrecords, (jobject)records_array);

   ret = 0;

end:
   if(is_chunked)
      free(is_chunked);
   if(records)
      free(records);
   e->ReleaseByteArrayElements(array, raw_msg, JNI_ABORT);

   return ret;
}

/*
 * JNI registration.
 */
static JNINativeMethod gMethods[] =
{
   {"parseNdefMessage", "([B)I",
      (void *)com_trustedlogic_trustednfc_android_NdefMessage_parseNdefMessage},
};

int register_com_trustedlogic_trustednfc_android_NdefMessage(JNIEnv *e)
{
   return jniRegisterNativeMethods(e,
      "com/trustedlogic/trustednfc/android/NdefMessage",
      gMethods, NELEM(gMethods));
}

} // namespace android
