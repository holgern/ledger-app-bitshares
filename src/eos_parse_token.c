/*******************************************************************************
*
*  This file is a derivative work, and contains modifications from original
*  form.  The modifications are copyright of their respective contributors,
*  and are licensed under the same terms as the original work.
*
*  Portions Copyright (c) 2019 Christopher J. Sanborn
*
*  Original copyright and license notice follows:
*
*   Taras Shchybovyk
*   (c) 2018 Taras Shchybovyk
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "eos_parse_token.h"
#include "eos_types.h"
#include "os.h"

// EOS version - DEPRECATED
void parseTokenTransfer(uint8_t *buffer, uint32_t bufferLength, uint8_t argNum, actionArgument_t *arg) {
    uint32_t read = 0;
    uint32_t written = 0;

    if (argNum == 0) {
        parseNameField(buffer, bufferLength, "From", arg, &read, &written);
    } else if (argNum == 1) {
        buffer += sizeof(name_t); bufferLength -= sizeof(name_t);
        parseNameField(buffer, bufferLength, "To", arg, &read, &written);
    } else if (argNum == 2) {
        buffer += 2 * sizeof(name_t); bufferLength -= 2 * sizeof(name_t);
        parseAssetField(buffer, bufferLength, "Quantity", arg, &read, &written);
    } else if (argNum == 3) {
        buffer += 2 * sizeof(name_t) + sizeof(asset_t); bufferLength -= 2 * sizeof(name_t) + sizeof(asset_t);
        parseStringField(buffer, bufferLength, "Memo", arg, &read, &written);
    }
}

/**
 * This is for testing with dummy arguments given as string literals.  Should be removed after.
*/
void parseStringLiteral(const char fieldText[], const char fieldName[], actionArgument_t *arg) {
    uint32_t labelLength = strlen(fieldName);
    uint32_t fieldTextLength = strlen(fieldText);
    if (labelLength > sizeof(arg->label) - 1) {
        PRINTF("parseOperationData Label too long\n");
        THROW(EXCEPTION);
    }
    if (fieldTextLength > sizeof(arg->data) - 1) {
        PRINTF("parseOperationData Insufficient buffer\n");
        THROW(EXCEPTION);
    }

    os_memset(arg->label, 0, sizeof(arg->label));
    os_memset(arg->data, 0, sizeof(arg->data));

    os_memmove(arg->label, fieldName, labelLength);
    os_memmove(arg->data, fieldText, fieldTextLength);

}

void parseTransferOperation(uint8_t *buffer, uint32_t bufferLength, uint8_t argNum, actionArgument_t *arg) {
    uint32_t read = 0;
    uint32_t written = 0;

    if (argNum == 0) {  // TEMP dummy args for now TODO: extract actual args
        parseStringLiteral("Transfer", "Operation", arg);
    } else if (argNum == 1) {
        parseStringLiteral("Somebody", "From", arg);
    } else if (argNum == 2) {
        parseStringLiteral("Somebody Else", "To", arg);
    } else if (argNum == 3) {
        parseStringLiteral("1000000 satoshis", "Amount", arg);
    }
}