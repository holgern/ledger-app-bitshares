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

#include "bts_parse_operations.h"
#include "bts_op_transfer.h"
#include "bts_op_limit_order_create.h"
#include "bts_op_limit_order_cancel.h"
#include "bts_op_account_update.h"
#include "bts_op_account_upgrade.h"
#include "bts_types.h"
#include "app_ui_displays.h"
#include "eos_utils.h"
#include "os.h"
#include <string.h>

/**
 * Shortcuts for printing into the txContent Display buffers:
 */
#define printfContentLabel(...) snprintf(ui_buffers.sign_tx.paramLabel, sizeof(ui_buffers.sign_tx.paramLabel), __VA_ARGS__)
#define printfContentParam(...) snprintf(ui_buffers.sign_tx.paramValue, sizeof(ui_buffers.sign_tx.paramValue), __VA_ARGS__)
#define WITH_SIZE(x) x, sizeof(x)

/**
 * Global Resource: User-friendly Operation names.  These can be indexed by the
 * operationId enum in bts_stream.h.  E.g. op_name[OP_TRANSFER] should resolve to
 * a pointer to "Transfer".
 */
const char * const op_names[] = {   // Deref with PIC or you gone get bit. E.g.:
    "Transfer",                     // const char * string = PIC(op_names[idx]);
    "Limit Order",                  // Otherwise string gonna point to the dark
    "Cancel Order",                 // and inky abyss.
    "Adjust Collateral",
    "fill_order", /* virtual */
    "Register Account",
    "Update Acct",
    "Whitelist Account",
    "Upgrade Acct",
    "Transfer Account Ownership",
    "Create Asset",
    "asset_update",
    "asset_update_bitasset",
    "asset_update_feed_producers",
    "asset_issue",
    "asset_reserve",
    "asset_fund_fee_pool",
    "asset_settle",
    "asset_global_settle",
    "asset_publish_feed",
    "witness_create",
    "witness_update",
    "proposal_create",
    "proposal_update",
    "proposal_delete",
    "withdraw_permission_create",
    "withdraw_permission_update",
    "withdraw_permission_claim",
    "withdraw_permission_delete",
    "committee_member_create",
    "committee_member_update",
    "committee_member_update_global_parameters",
    "vesting_balance_create",
    "vesting_balance_withdraw",
    "worker_create",
    "custom",
    "assert",
    "balance_claim",
    "override_transfer",
    "transfer_to_blind",
    "blind_transfer",
    "transfer_from_blind",
    "asset_settle_cancel", /* virtual */
    "asset_claim_fees",
    "fba_distribute", /* virtual */
    "bid_collateral",
    "execute_bid", /* virtual */
    "asset_claim_pool",
    "asset_update_issuer"
};

void updateOperationContent() {

    const operationId_t opId = txContent.operationIds[txContent.currentOperation];

    const char * opName = "";
    if (opId < OP_NUM_KNOWN_OPS) {
        opName = (const char *)PIC(op_names[opId]); // PIC or you gonna bleed...
    }

    switch (opId) {
    case OP_TRANSFER:
        txContent.argumentCount = 4;
        txContent.operationParser = parseTransferOperation;
        break;
    case OP_LIMIT_ORDER_CREATE:
        txContent.argumentCount = 6;
        txContent.operationParser = parseLimitOrderCreateOperation;
        break;
    case OP_LIMIT_ORDER_CANCEL:
        txContent.argumentCount = 3;
        txContent.operationParser = parseLimitOrderCancelOperation;
        break;
    case OP_CALL_ORDER_UPDATE:  /* Unsupport */
    case OP_FILL_ORDER:         /* Unsupport */ /* virtual */
    case OP_ACCOUNT_CREATE:     /* Unsupport */
        txContent.argumentCount = 2;
        txContent.operationParser = parseUnsupportedOperation;
        break;
    case OP_ACCOUNT_UPDATE:
        txContent.argumentCount = 5;
        txContent.operationParser = parseAccountUpdateOperation;
        break;
    case OP_ACCOUNT_WHITELIST:  /* Unsupport */
        txContent.argumentCount = 2;
        txContent.operationParser = parseUnsupportedOperation;
        break;
    case OP_ACCOUNT_UPGRADE:
        txContent.argumentCount = 3;
        txContent.operationParser = parseAccountUpgradeOperation;
        break;
    case OP_ACCOUNT_TRANSFER:   /* Unsupport */
    case OP_ASSET_CREATE:       // ...
    case OP_ASSET_UPDATE:
    case OP_ASSET_UPDATE_BITASSET:
    case OP_ASSET_UPDATE_FEED_PRODUCERS:
    case OP_ASSET_ISSUE:
    case OP_ASSET_RESERVE:
    case OP_ASSET_FUND_FEE_POOL:
    case OP_ASSET_SETTLE:
    case OP_ASSET_GLOBAL_SETTLE:
    case OP_ASSET_PUBLISH_FEED:
    case OP_WITNESS_CREATE:
    case OP_WITNESS_UPDATE:
    case OP_PROPOSAL_CREATE:
    case OP_PROPOSAL_UPDATE:
    case OP_PROPOSAL_DELETE:
    case OP_WITHDRAW_PERMISSION_CREATE:
    case OP_WITHDRAW_PERMISSION_UPDATE:
    case OP_WITHDRAW_PERMISSION_CLAIM:
    case OP_WITHDRAW_PERMISSION_DELETE:
    case OP_COMMITTEE_MEMBER_CREATE:
    case OP_COMMITTEE_MEMBER_UPDATE:
    case OP_COMMITTEE_MEMBER_UPDATE_GLOBAL_PARAMETERS:
    case OP_VESTING_BALANCE_CREATE:
    case OP_VESTING_BALANCE_WITHDRAW:
    case OP_WORKER_CREATE:
    case OP_CUSTOM:
    case OP_ASSERT:
    case OP_BALANCE_CLAIM:
    case OP_OVERRIDE_TRANSFER:
    case OP_TRANSFER_TO_BLIND:
    case OP_BLIND_TRANSFER:
    case OP_TRANSFER_FROM_BLIND:
    case OP_ASSET_SETTLE_CANCEL: /* virtual */
    case OP_ASSET_CLAIM_FEES:
    case OP_FBA_DISTRIBUTE:      /* virtual */
    case OP_BID_COLLATERAL:
    case OP_EXECUTE_BID:         /* virtual */
    case OP_ASSET_CLAIM_POOL:
    case OP_ASSET_UPDATE_ISSUER:
        txContent.argumentCount = 2;
        txContent.operationParser = parseUnsupportedOperation;
        break;
    default:
        txContent.argumentCount = 2;
        txContent.operationParser = parseUnknownOperation;
        opName = "** Unknown Operation **";
        break;
    }

    os_memset(ui_buffers.sign_tx.paramValue, 0, sizeof(ui_buffers.sign_tx.paramValue));
    os_memmove(ui_buffers.sign_tx.paramValue, opName,
               MIN(sizeof(ui_buffers.sign_tx.paramValue)-1,strlen(opName)));
    os_memset(ui_buffers.sign_tx.paramLabel, 0, sizeof(ui_buffers.sign_tx.paramLabel));
    snprintf(ui_buffers.sign_tx.paramLabel, sizeof(ui_buffers.sign_tx.paramLabel),
             "Operation %u of %u", txContent.currentOperation+1, txContent.operationCount);

}

void parseTransferOperation(const uint8_t *buffer, uint32_t bufferLength, uint8_t argNum) {
    uint32_t read = 0;
    bts_operation_transfer_t op;

    // Read fields:
    read += deserializeBtsOperationTransfer(buffer, bufferLength, &op);

    if (argNum == 0) {
        printfContentLabel("Amount");
        prettyPrintBtsAssetType(op.transferAsset, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 1) {
        printfContentLabel("From");
        prettyPrintBtsAccountIdType(op.fromId, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 2) {
        printfContentLabel("To");
        prettyPrintBtsAccountIdType(op.toId, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 3) {
        printfContentLabel("Fee");
        prettyPrintBtsAssetType(op.feeAsset, ui_buffers.sign_tx.paramValue);
    }
}

void parseLimitOrderCreateOperation(const uint8_t *buffer, uint32_t bufferLength, uint8_t argNum) {
    uint32_t read = 0;
    bts_operation_limit_order_create_t op;

    // Read fields:
    read += deserializeBtsOperationLimitOrderCreate(buffer, bufferLength, &op);

    if (argNum == 0) {
        printfContentLabel("Seller");
        prettyPrintBtsAccountIdType(op.sellerId, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 1) {
        printfContentLabel("Amount to Sell");
        prettyPrintBtsAssetType(op.sellAsset, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 2) {
        printfContentLabel("Amount to Buy");
        prettyPrintBtsAssetType(op.buyAsset, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 3) {
        printfContentLabel("Expires");
        prettyPrintBtsTimeType(op.expires, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 4) {
        printfContentLabel("Fill or Kill");
        prettyPrintBtsBoolType(op.fillOrKill, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 5) {
        printfContentLabel("Fee");
        prettyPrintBtsAssetType(op.feeAsset, ui_buffers.sign_tx.paramValue);
    }
}

void parseLimitOrderCancelOperation(const uint8_t *buffer, uint32_t bufferLength, uint8_t argNum) {
    uint32_t read = 0;
    bts_operation_limit_order_cancel_t op;

    // Read fields:
    read += deserializeBtsOperationLimitOrderCancel(buffer, bufferLength, &op);

    if (argNum == 0) {
        printfContentLabel("Account");
        prettyPrintBtsAccountIdType(op.accountId, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 1) {
        printfContentLabel("Order Id");
        ui64toa(op.orderId, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 2) {
        printfContentLabel("Fee");
        prettyPrintBtsAssetType(op.feeAsset, ui_buffers.sign_tx.paramValue);
    }
}

void parseAccountUpdateOperation(const uint8_t *buffer, uint32_t bufferLength, uint8_t argNum) {
    uint32_t read = 0;
    bts_operation_account_update_t op;

    // Read fields:
    read += deserializeBtsOperationAccountUpdate(buffer, bufferLength, &op);

    if (argNum == 0) {
        printfContentLabel("Account to Update");
        prettyPrintBtsAccountIdType(op.accountId, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 1) {
        bool permPresent = op.ownerPermissionPresent;
        bts_permission_type_t * perm = &op.ownerPermission;
        if (txContent.subargRemainP1 == 0) {
            txContent.subargRemainP1 = permPresent ? 6 : 0;
            printfContentLabel("Owner Permission");
            printfContentParam(permPresent?"New Data":"No Change");
        } else if (txContent.subargRemainP1 == 5) {
            printfContentLabel("Owner Threshold");
            printfContentParam("%u", (unsigned int)perm->weightThreshold);
        } else if (txContent.subargRemainP1 == 4) {
            printfContentLabel("Account Auths");
            printfContentParam("Count: %u", perm->numAccountAuths);
        } else if (txContent.subargRemainP1 == 3) {
            printfContentLabel("Account Auths");
            prettyPrintBtsAccountAuthsList(*perm, WITH_SIZE(ui_buffers.sign_tx.paramValue));
        } else if (txContent.subargRemainP1 == 2) {
            printfContentLabel("Key Auths");
            printfContentParam("Count: %u", perm->numKeyAuths);
        } else if (txContent.subargRemainP1 == 1) {
            printfContentLabel("Key Auths");
            prettyPrintBtsKeyAuthsList(*perm, WITH_SIZE(ui_buffers.sign_tx.paramValue));
        }
    } else if (argNum == 2) {
        bool permPresent = op.activePermissionPresent;
        bts_permission_type_t * perm = &op.activePermission;
        if (txContent.subargRemainP1 == 0) {
            txContent.subargRemainP1 = permPresent ? 6 : 0;
            printfContentLabel("Active Permission");
            printfContentParam(permPresent?"New Data":"No Change");
        } else if (txContent.subargRemainP1 == 5) {
            printfContentLabel("Active Threshold");
            printfContentParam("%u", (unsigned int)perm->weightThreshold);
        } else if (txContent.subargRemainP1 == 4) {
            printfContentLabel("Account Auths");
            printfContentParam("Count: %u", perm->numAccountAuths);
        } else if (txContent.subargRemainP1 == 3) {
            printfContentLabel("Account Auths");
            prettyPrintBtsAccountAuthsList(*perm, WITH_SIZE(ui_buffers.sign_tx.paramValue));
        } else if (txContent.subargRemainP1 == 2) {
            printfContentLabel("Key Auths");
            printfContentParam("Count: %u", perm->numKeyAuths);
        } else if (txContent.subargRemainP1 == 1) {
            printfContentLabel("Key Auths");
            prettyPrintBtsKeyAuthsList(*perm, WITH_SIZE(ui_buffers.sign_tx.paramValue));
        }
    } else if (argNum == 3) {
        bts_account_options_type_t * opts = &op.accountOptions;
        if (txContent.subargRemainP1 == 0) {
            txContent.subargRemainP1 = op.accountOptionsPresent ? 7 : 0;
            printfContentLabel("Account Options");
            printfContentParam(op.accountOptionsPresent?"New Data":"No Change");
        } else if (txContent.subargRemainP1 == 6) {
            printfContentLabel("Memo Public Key");
            prettyPrintBtsPublicKeyType(opts->memoPubkey, ui_buffers.sign_tx.paramValue);
        } else if (txContent.subargRemainP1 == 5) {
            printfContentLabel("Voting Account");
            prettyPrintBtsAccountIdType(opts->votingAccount, ui_buffers.sign_tx.paramValue);
        } else if (txContent.subargRemainP1 == 4) {
            printfContentLabel("Num Witnesses");
            printfContentParam("%u", (unsigned int)opts->numWitnesses);
        } else if (txContent.subargRemainP1 == 3) {
            printfContentLabel("Num Committee");
            printfContentParam("%u", (unsigned int)opts->numCommittee);
        } else if (txContent.subargRemainP1 == 2) {
            printfContentLabel("Votes");
            printfContentParam("Count: %u", opts->numVotes);
        } else if (txContent.subargRemainP1 == 1) {
            printfContentLabel("Votes");
            prettyPrintBtsVotesList(*opts, WITH_SIZE(ui_buffers.sign_tx.paramValue));
        }
    } else if (argNum == 4) {
        printfContentLabel("Fee");
        prettyPrintBtsAssetType(op.feeAsset, ui_buffers.sign_tx.paramValue);
    }
}

void parseAccountUpgradeOperation(const uint8_t *buffer, uint32_t bufferLength, uint8_t argNum) {
    uint32_t read = 0;
    bts_operation_account_upgrade_t op;

    // Read fields:
    read += deserializeBtsOperationAccountUpgrade(buffer, bufferLength, &op);

    if (argNum == 0) {
        printfContentLabel("Account to Upgrade");
        prettyPrintBtsAccountIdType(op.accountId, ui_buffers.sign_tx.paramValue);
    } else if (argNum == 1) {
        printfContentLabel("Upgrade Path");
        printfContentParam(op.upgradeLtm?"Lifetime Membership":"None / No Upgrade");
    } else if (argNum == 2) {
        printfContentLabel("Fee");
        prettyPrintBtsAssetType(op.feeAsset, ui_buffers.sign_tx.paramValue);
    }
}

void parseUnsupportedOperation(const uint8_t *buffer, uint32_t bufferLength, uint8_t argNum) {

    if (argNum == 0) {
        printfContentLabel("Unsupported");
        printfContentParam("Operation");
    } else if (argNum == 1) {
        printfContentLabel("Confirm at");
        printfContentParam("Own Risk.");
    }
}

void parseUnknownOperation(const uint8_t *buffer, uint32_t bufferLength, uint8_t argNum) {

    if (argNum == 0) {
        printfContentLabel("Unrecognized");
        printfContentParam("Operation");
    } else if (argNum == 1) {
        printfContentLabel("Confirm at");
        printfContentParam("Own Risk.");
    }
}
