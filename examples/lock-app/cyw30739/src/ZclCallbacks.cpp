/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "LockManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInstanceInfoProvider.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

void emberAfBasicClusterInitCallback(EndpointId endpoint)
{
    uint16_t year;
    uint8_t month;
    uint8_t dayOfMonth;
    char cString[16] = "00000000";

    if (GetDeviceInstanceInfoProvider()->GetManufacturingDate(year, month, dayOfMonth) == CHIP_NO_ERROR)
    {
        snprintf(cString, sizeof(cString), "%04u%02u%02u", year, month, dayOfMonth);
    }
    Basic::Attributes::ManufacturingDate::Set(endpoint, CharSpan(cString));
}

void MatterPostAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)

{
    switch (attributePath.mClusterId)
    {
    case DoorLock::Id:
        if (attributePath.mAttributeId == DoorLock::Attributes::LockState::Id)
        {
            ChipLogProgress(Zcl, "Door lock cluster: " ChipLogFormatMEI, ChipLogValueMEI(attributePath.mClusterId));
            printf("ZCL OnOff -> %u\n", *value);
            LockMgr().InitiateAction(LockManager::ACTOR_ZCL_CMD, (*value) ? LockManager::LOCK_ACTION : LockManager::UNLOCK_ACTION);
            return;
        }
        break;
    default:
        printf("Unhandled cluster ID: 0x%04lx\n", attributePath.mClusterId);
        return;
    }

    printf("ERROR clusterId: 0x%04lx, unknown attribute ID: 0x%04lx\n", attributePath.mClusterId, attributePath.mAttributeId);
}

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    ChipLogProgress(Zcl, "Door Lock App: Lock Command endpoint=%d", endpointId);
    bool status = LockMgr().Lock(endpointId, pinCode, err);
    if (status == true)
    {
        LockMgr().InitiateAction(AppEvent::kEventType_Lock, LockManager::LOCK_ACTION);
    }
    return status;
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode,
                                              DlOperationError & err)
{
    ChipLogProgress(Zcl, "Door Lock App: Unlock Command endpoint=%d", endpointId);
    bool status = LockMgr().Unlock(endpointId, pinCode, err);
    if (status == true)
    {
        LockMgr().InitiateAction(AppEvent::kEventType_Lock, LockManager::UNLOCK_ACTION);
    }

    return status;
}

bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialType credentialType,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    return LockMgr().GetCredential(endpointId, credentialIndex, credentialType, credential);
}

bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialStatus credentialStatus,
                                        DlCredentialType credentialType, const chip::ByteSpan & credentialData)
{
    return LockMgr().SetCredential(endpointId, credentialIndex, credentialStatus, credentialType, credentialData);
}

bool emberAfPluginDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    return LockMgr().GetUser(userIndex, user);
}

bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator,
                                  chip::FabricIndex modifier, const chip::CharSpan & userName, uint32_t uniqueId,
                                  DlUserStatus userStatus, DlUserType usertype, DlCredentialRule credentialRule,
                                  const DlCredential * credentials, size_t totalCredentials)
{

    return LockMgr().SetUser(userIndex, creator, modifier, userName, uniqueId, userStatus, usertype, credentialRule, credentials,
                             totalCredentials);
}

// TODO: These functions will be supported by door-lock-server in the future. These are set to return failure until implemented.
DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockWeekDaySchedule & schedule)
{
    return DlStatus::kFailure;
}

DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    return DlStatus::kFailure;
}

DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, DlDaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                          uint8_t endHour, uint8_t endMinute)
{
    return DlStatus::kFailure;
}

DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime)
{
    return DlStatus::kFailure;
}
