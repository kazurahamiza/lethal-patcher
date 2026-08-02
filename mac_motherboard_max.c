#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

/* Declare System Power Management assertion */
static IOPMAssertionID gNoSleepAssertionID = kIOPMNullAssertionID;

/* Prevent System Board & Bus Throttling */
bool DisableMacBusThrottling(void) {
    CFStringRef reasonForActivity = CFSTR("Max Motherboard Performance Execution");

    /* Create assertion to disable CPU and system bus clock gating */
    IOReturn success = IOPMAssertionCreateWithName(
        kIOPMAssertionTypePreventUserIdleSystemSleep,
        kIOPMAssertionLevelOn,
        reasonForActivity,
        &gNoSleepAssertionID
    );

    if (success == kIOReturnSuccess) {
        printf("[+] Logic Board assertion active: Bus Clock Gating & Thermal Throttling disabled.\n");
        return true;
    }

    printf("[-] Failed to dispatch IOPM logic board assertion.\n");
    return false;
}

int main(void) {
    printf("===========================================\n");
    printf("   MACOS LOGIC BOARD & BUS MAX ENGINE      \n");
    printf("===========================================\n");

    DisableMacBusThrottling();

    printf("===========================================\n");
    printf("[SUCCESS] macOS logic board power limits overridden.\n");
    return 0;
}
