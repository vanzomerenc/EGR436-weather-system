/*
 * sdcard.h
 *
 *  Created on: Feb 19, 2018
 *      Author: Samanth
 */

#ifndef SDCARD_H_
#define SDCARD_H_

#include <stdio.h>
#include <stdbool.h>
#include <driverlib.h>
#include <string.h>
#include <ctype.h>
#include "drv/rtc.h"
#include "drv/spiDriver.h"
#include "utils/cmdline.h"
#include "third_party/fatfs/src/ff.h"
#include "third_party/fatfs/src/diskio.h"

// Defines the size of the buffers that hold the path, or temporary data from
// the SD card.  There are two buffers allocated of this size.  The buffer size
// must be large enough to hold the longest expected full path name, including
// the file name, and a trailing null character.
#define PATH_BUF_SIZE           80

// Defines the size of the buffer that holds the command line.
#define CMD_BUF_SIZE            64
// This buffer holds the full path to the current working directory.  Initially
// it is root ("/").
static char g_pcCwdBuf[PATH_BUF_SIZE] = "/";

// A temporary data buffer used when manipulating file paths, or reading data
// from the SD card.
static char g_pcTmpBuf[PATH_BUF_SIZE];

// The buffer that holds the command line.
static char g_pcCmdBuf[CMD_BUF_SIZE];

// The following are data structures used by FatFs.
static FATFS g_sFatFs;
static DIR g_sDirObject;
static FILINFO g_sFileInfo;
static FIL g_sFileObject;

// Command declaration.
int Cmd_help(int argc, char *argv[]);
int Cmd_ls(int argc, char *argv[]);
int Cmd_pwd(int argc, char *argv[]);
int Cmd_cd(int argc, char *argv[]);
int Cmd_cat(int argc, char *argv[]);

//*****************************************************************************
//
// This is the table that holds the command names, implementing functions, and
// brief description.
//
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] = {
        { "help", Cmd_help, "Display list of commands" }, { "h", Cmd_help,
                "alias for help" }, { "?", Cmd_help, "alias for help" }, { "ls",
                Cmd_ls, "Display list of files" }, { "chdir", Cmd_cd,
                "Change directory" }, { "cd", Cmd_cd, "alias for chdir" }, {
                "pwd", Cmd_pwd, "Show current working directory" }, { "cat",
                Cmd_cat, "Show contents of a text file" }, { 0, 0, 0 } };

// A structure that holds a mapping between an FRESULT numerical code, and a
// string representation.  FRESULT codes are returned from the FatFs FAT file
// system driver.
typedef struct {
    FRESULT iFResult;
    char *pcResultStr;
} tFResultString;

// A macro to make it easy to add result codes to the table.
#define FRESULT_ENTRY(f)        { (f), (#f) }

// A table that holds a mapping between the numerical FRESULT code and it's
// name as a string.  This is used for looking up error codes for printing to
// the console.
tFResultString g_psFResultStrings[] = {
FRESULT_ENTRY(FR_OK),
FRESULT_ENTRY(FR_DISK_ERR),
FRESULT_ENTRY(FR_INT_ERR),
FRESULT_ENTRY(FR_NOT_READY),
FRESULT_ENTRY(FR_NO_FILE),
FRESULT_ENTRY(FR_NO_PATH),
FRESULT_ENTRY(FR_INVALID_NAME),
FRESULT_ENTRY(FR_DENIED),
FRESULT_ENTRY(FR_EXIST),
FRESULT_ENTRY(FR_INVALID_OBJECT),
FRESULT_ENTRY(FR_WRITE_PROTECTED),
FRESULT_ENTRY(FR_INVALID_DRIVE),
FRESULT_ENTRY(FR_NOT_ENABLED),
FRESULT_ENTRY(FR_NO_FILESYSTEM),
FRESULT_ENTRY(FR_MKFS_ABORTED),
FRESULT_ENTRY(FR_TIMEOUT),
FRESULT_ENTRY(FR_LOCKED),
FRESULT_ENTRY(FR_NOT_ENOUGH_CORE),
FRESULT_ENTRY(FR_TOO_MANY_OPEN_FILES),
FRESULT_ENTRY(FR_INVALID_PARAMETER), };

// A macro that holds the number of result codes.
#define NUM_FRESULT_CODES       (sizeof(g_psFResultStrings) /                 \
                                 sizeof(tFResultString))

uint8_t gucCommandReady = 0;
// This function returns a string representation of an error code that was
// returned from a function call to FatFs.  It can be used for printing human
// readable error messages.
const char *
StringFromFResult(FRESULT iFResult) {
    uint_fast8_t ui8Idx;

    // Enter a loop to search the error code table for a matching error code.
    for (ui8Idx = 0; ui8Idx < NUM_FRESULT_CODES; ui8Idx++) {
        // If a match is found, then return the string name of the error code.
        if (g_psFResultStrings[ui8Idx].iFResult == iFResult) {
            return (g_psFResultStrings[ui8Idx].pcResultStr);
        }
    }

    // At this point no matching code was found, so return a string indicating
    // an unknown error.
    return ("UNKNOWN ERROR CODE");
}

void SysTick_ISR(void) {
    // Call the FatFs tick timer.
    disk_timerproc();
}


enum TimeSetState { NOT_SETTING, SETTING_MONTH, SETTING_DAY, SETTING_YEAR, SETTING_HOUR, SETTING_MINUTE, SETTING_RATE, DONE_SETTING };
enum TimeSetState timeSetState = NOT_SETTING;

char UARTBuffer[256] = {'\0'};
char newInput[256] = {'\0'};
int newInputReceived = 0;

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://processors.wiki.ti.com/index.php/
 *               USCI_UART_Baud_Rate_Gen_Mode_Selection
 */
const eUSCI_UART_Config uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        208,                                      // BRDIV = 26
        0,                                       // UCxBRF = 0
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // MSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION  // Low Frequency Mode
};

/*
 * USCIA0 interrupt handler.
 */
void EusciA0_ISR(void)
{
    int receiveByte = UCA0RXBUF;

    // Will use this as "enter" for the user.
    if(receiveByte == 13) // This is sent from a carriage return
    {
        // Set flag indicating new input available in buffer
        newInputReceived = 1;
        // Copy the buffer to the newInput global string so we don't lose it
        strcpy(newInput, UARTBuffer);
        // Set the first byte of buffer to a null char so we can start a new string
        UARTBuffer[0] = '\0';
        //__no_operation();
    }
    else
    {
        // Check for valid input
        if(isdigit(receiveByte))
        {
            // Convert character to null-terminating string
            char tempS[2];
            tempS[0] = receiveByte;
            tempS[1] = '\0';
            // Concat the char string with the bytes already stored in buffer
            strcat(UARTBuffer, tempS);
        }
    }

    /* Echo back. */
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, receiveByte);
}

// For tera term communication using the UART backchannel
void initUART()
{
    /* Selecting P1.2 and P1.3 in UART mode. */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
        GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_BASE);

    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT | EUSCI_A_UART_BREAKCHAR_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);
}

void processUART(struct rtc_time* time)
{
    // This tracks a state machine that keeps track of where the user
    // is in the process of setting the date/time
    int interval;
    if(newInputReceived && timeSetState != DONE_SETTING)
    {
        switch(timeSetState)
        {
        // first prompt
            case NOT_SETTING:
                printf("\n\rEnter the month\n\r");
                break;
            case SETTING_MONTH: // Grab int from serial in, stored as month
                sscanf(newInput, "%d", &time->month);
                printf("\n\rEnter the day\n\r");
                break;
            case SETTING_DAY: // Store int from serial in as day of month
                sscanf(newInput, "%d", &time->date);
                printf("\n\rEnter the year\n\r");
                break;
            case SETTING_YEAR: // Store int from serial in as year
                sscanf(newInput, "%d", &time->year);
                printf("\n\rEnter the hour\n\r");
                break;
            case SETTING_HOUR:  // hour
                sscanf(newInput, "%d", &time->hour);
                printf("\n\rEnter the minute\n\r");
                break;
            case SETTING_MINUTE: // minutes
                sscanf(newInput, "%d", &time->min);
                printf("\n\rEnter the collection rate (0 = per second, 1 = per minute, 2 = per hour\n\r");
                break;
            case SETTING_RATE: // rate to collect data
                // TODO Implement this feature to set the rate of data collection
                sscanf(newInput, "%d", &interval);
                rtc_setinterval(interval);
                if(rtc_getinterval() == POLL_MINUTE)
                {
                    MAP_RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);
                }
                if(rtc_getinterval() == POLL_HOUR)
                {
                    MAP_RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_HOURCHANGE);
                }
                printf("\n\rTime set.\n\r");
                break;

        }
        if(timeSetState == SETTING_RATE)
        {
            // If we got to the last stage, go ahead and send the struct on
            rtc_settime(time);
            // Print formatted results to terminal
            printf("%d-%d-%d %d:%d:%d\n\r", time->month
                   , time->date, time->year, time->hour
                   , time->min, time->sec);
            // Set the state machine to stop
            timeSetState = DONE_SETTING;
        }
        else
        {
            ++timeSetState;
        }
        newInputReceived = 0;
    }
}
//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int Cmd_help(int argc, char *argv[]) {
    tCmdLineEntry *psEntry;

    // Print some header text.
    printf("\nAvailable commands\r\n");
    printf("------------------\r\n");

    // Point at the beginning of the command table.
    psEntry = &g_psCmdTable[0];

    // Enter a loop to read each entry from the command table.  The end of the
    // table has been reached when the command name is NULL.
    while (psEntry->pcCmd) {
        // Print the command name and the brief description.
        printf("%6s: %s\r\n", psEntry->pcCmd, psEntry->pcHelp);

        // Advance to the next entry in the table.
        psEntry++;
    }

    // Return success.
    return (0);
}

//*****************************************************************************
//
// This function implements the "ls" command.  It opens the current directory
// and enumerates through the contents, and prints a line for each item it
// finds.  It shows details such as file attributes, time and date, and the
// file size, along with the name.  It shows a summary of file sizes at the end
// along with free space.
//
//*****************************************************************************
int Cmd_ls(int argc, char *argv[]) {
    uint32_t ui32TotalSize;
    uint32_t ui32FileCount;
    uint32_t ui32DirCount;
    FRESULT iFResult;
    FATFS *psFatFs;
    char *pcFileName;
#if _USE_LFN
    char pucLfn[_MAX_LFN + 1];
    g_sFileInfo.lfname = pucLfn;
    g_sFileInfo.lfsize = sizeof(pucLfn);
#endif

    //
    // Open the current directory for access.
    //
    iFResult = f_opendir(&g_sDirObject, g_pcCwdBuf);

    //
    // Check for error and return if there is a problem.
    //
    if (iFResult != FR_OK) {
        return ((int) iFResult);
    }

    ui32TotalSize = 0;
    ui32FileCount = 0;
    ui32DirCount = 0;

    //
    // Give an extra blank line before the listing.
    //
    printf("\r\n");

    //
    // Enter loop to enumerate through all directory entries.
    //
    for (;;) {
        //
        // Read an entry from the directory.
        //
        iFResult = f_readdir(&g_sDirObject, &g_sFileInfo);

        //
        // Check for error and return if there is a problem.
        //
        if (iFResult != FR_OK) {
            return ((int) iFResult);
        }

        //
        // If the file name is blank, then this is the end of the listing.
        //
        if (!g_sFileInfo.fname[0]) {
            break;
        }

        //
        // If the attribue is directory, then increment the directory count.
        //
        if (g_sFileInfo.fattrib & AM_DIR) {
            ui32DirCount++;
        }

        //
        // Otherwise, it is a file.  Increment the file count, and add in the
        // file size to the total.
        //
        else {
            ui32FileCount++;
            ui32TotalSize += g_sFileInfo.fsize;
        }

#if _USE_LFN
        pcFileName = (
                (*g_sFileInfo.lfname) ? g_sFileInfo.lfname : g_sFileInfo.fname);
#else
        pcFileName = g_sFileInfo.fname;
#endif
        //
        // Print the entry information on a single line with formatting to show
        // the attributes, date, time, size, and name.
        //
        printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %s\r\n",
                (g_sFileInfo.fattrib & AM_DIR) ? 'D' : '-',
                (g_sFileInfo.fattrib & AM_RDO) ? 'R' : '-',
                (g_sFileInfo.fattrib & AM_HID) ? 'H' : '-',
                (g_sFileInfo.fattrib & AM_SYS) ? 'S' : '-',
                (g_sFileInfo.fattrib & AM_ARC) ? 'A' : '-',
                (g_sFileInfo.fdate >> 9) + 1980, (g_sFileInfo.fdate >> 5) & 15,
                g_sFileInfo.fdate & 31, (g_sFileInfo.ftime >> 11),
                (g_sFileInfo.ftime >> 5) & 63, g_sFileInfo.fsize, pcFileName);
    }

    //
    // Print summary lines showing the file, dir, and size totals.
    //
    printf("\n%4u File(s),%10u bytes total\r\n%4u Dir(s)", ui32FileCount,
            ui32TotalSize, ui32DirCount);

    //
    // Get the free space.
    //
    iFResult = f_getfree("/", (DWORD *) &ui32TotalSize, &psFatFs);

    //
    // Check for error and return if there is a problem.
    //
    if (iFResult != FR_OK) {
        return ((int) iFResult);
    }

    //
    // Display the amount of free space that was calculated.
    //
    //printf(", %10uK bytes free\r\n", (ui32TotalSize * psFatFs->free_clust / 2));
    // Not sure about this yet. Removing ui32TotalSize give the correct value.
    printf(", %10uK bytes free\r\n", ( psFatFs->free_clust / 2));

    //
    // Made it to here, return with no errors.
    //
    return (0);
}

// File name used in writing to the SD card
char FILE_NAME[17] = "record";

FRESULT closeSD()
{
    FRESULT iFResult;
    iFResult = f_close(&g_sFileObject);
    if(iFResult != FR_OK) {
        printf("f_close error: %s\n", StringFromFResult(iFResult));
    }
    return iFResult;
}

// Taken from the main.c function of the SD card library's example program
// Modified for project 3-6-2018 ST
int initSD()
{
    //int8_t lucNStatus = 0;
    FRESULT iFResult;

    /* Configure SysTick for a 100Hz interrupt.  The FatFs driver wants a 10 ms
     * tick.
     */
    SysTick_setPeriod(48000000 / 100);
    SysTick_enableModule();
    SysTick_enableInterrupt();

    spi_Open();

    // Print hello message to user.
    printf("\n\rInitializing SD card\r\n");
    //printf("Type \'help\' for help.\r\n");

    // Mount the file system, using logical disk 0.
    iFResult = f_mount(0, &g_sFatFs);
    //iFResult = f_mount(&g_sFatFs, "", 0);
    if (iFResult != FR_OK) {
        printf("f_mount error: %s\n\r", StringFromFResult(iFResult));
        return (1);
    }

    // Start the new file with a header for the CSV stuff
    unsigned int writeBytes = 0;
    char writeBuff[1000] = {'\0'};
    printf("%s\n\r", FILE_NAME);
    sprintf(writeBuff, "Minute,Hour,Day,Month,Year,Light,Temp,Humidity,Pressure\r\n");
    iFResult = f_open(&g_sFileObject, FILE_NAME, FA_WRITE | FA_CREATE_ALWAYS);
    if(iFResult != FR_OK)
    {
        printf("f_open error: %s\n\r", StringFromFResult(iFResult));
    }
    iFResult = f_write(&g_sFileObject, writeBuff,strlen(writeBuff),&writeBytes);
    if(iFResult != FR_OK)
    {
        printf("f_write error: %s\n\r", StringFromFResult(iFResult));
    }
    printf("%d characters written\n\r", writeBytes);
    closeSD();

    return writeBytes;
}
// Taken from the main.c function of the SD card library's example program
// Modified for project 3-6-2018 ST
int writeSD(uint8_t minute, uint8_t hour, uint8_t day, uint8_t month, uint16_t year
            , float light, float temp, float humidity, float pressure)
{
    unsigned int writeBytes = 0;
    char writeBuff[1000] = {'\0'};
    FRESULT iFResult;

    // Open existing file.
    iFResult = f_open(&g_sFileObject, FILE_NAME, FA_WRITE);
    if(iFResult != FR_OK)
    {
        printf("f_open error: %s\n", StringFromFResult(iFResult));
    }

    // Move file pointer to end of file
    iFResult = f_lseek(&g_sFileObject, g_sFileObject.fsize);
    if(iFResult != FR_OK)
    {
        printf("f_lseek error: %s\n\r", StringFromFResult(iFResult));
    }

    // Write new time/weather data to the file
    sprintf(writeBuff, "%.2d,%.2d,%.2d,%.2d,%.4d,%f,%f,%f,%f\r\n"
            , minute, hour, day, month, year, light, temp, humidity, pressure);
    iFResult = f_write(&g_sFileObject, writeBuff, strlen(writeBuff), &writeBytes);
    if(iFResult != FR_OK)
    {
        printf("f_write error: %s\n\r", StringFromFResult(iFResult));
    }
    printf("%d characters written\n\r", writeBytes);

    // Close the file
    closeSD();

    return writeBytes;
}



//*****************************************************************************
//
// This function implements the "cd" command.  It takes an argument that
// specifies the directory to make the current working directory.  Path
// separators must use a forward slash "/".  The argument to cd can be one of
// the following:
//
// * root ("/")
// * a fully specified path ("/my/path/to/mydir")
// * a single directory name that is in the current directory ("mydir")
// * parent directory ("..")
//
// It does not understand relative paths, so dont try something like this:
// ("../my/new/path")
//
// Once the new directory is specified, it attempts to open the directory to
// make sure it exists.  If the new path is opened successfully, then the
// current working directory (cwd) is changed to the new path.
//
//*****************************************************************************
int Cmd_cd(int argc, char *argv[]) {
    uint_fast8_t ui8Idx;
    FRESULT iFResult;

    //
    // Copy the current working path into a temporary buffer so it can be
    // manipulated.
    //
    strcpy(g_pcTmpBuf, g_pcCwdBuf);

    //
    // If the first character is /, then this is a fully specified path, and it
    // should just be used as-is.
    //
    if (argv[1][0] == '/') {
        //
        // Make sure the new path is not bigger than the cwd buffer.
        //
        if (strlen(argv[1]) + 1 > sizeof(g_pcCwdBuf)) {
            printf("Resulting path name is too long\r\n");
            return (0);
        }

        //
        // If the new path name (in argv[1])  is not too long, then copy it
        // into the temporary buffer so it can be checked.
        //
        else {
            strncpy(g_pcTmpBuf, argv[1], sizeof(g_pcTmpBuf));
        }
    }

    //
    // If the argument is .. then attempt to remove the lowest level on the
    // CWD.
    //
    else if (!strcmp(argv[1], "..")) {
        //
        // Get the index to the last character in the current path.
        //
        ui8Idx = strlen(g_pcTmpBuf) - 1;

        //
        // Back up from the end of the path name until a separator (/) is
        // found, or until we bump up to the start of the path.
        //
        while ((g_pcTmpBuf[ui8Idx] != '/') && (ui8Idx > 1)) {
            //
            // Back up one character.
            //
            ui8Idx--;
        }

        //
        // Now we are either at the lowest level separator in the current path,
        // or at the beginning of the string (root).  So set the new end of
        // string here, effectively removing that last part of the path.
        //
        g_pcTmpBuf[ui8Idx] = 0;
    }

    //
    // Otherwise this is just a normal path name from the current directory,
    // and it needs to be appended to the current path.
    //
    else {
        //
        // Test to make sure that when the new additional path is added on to
        // the current path, there is room in the buffer for the full new path.
        // It needs to include a new separator, and a trailing null character.
        //
        if (strlen(g_pcTmpBuf) + strlen(argv[1]) + 1 + 1 > sizeof(g_pcCwdBuf)) {
            printf("Resulting path name is too long\r\n");
            return (0);
        }

        //
        // The new path is okay, so add the separator and then append the new
        // directory to the path.
        //
        else {
            //
            // If not already at the root level, then append a /
            //
            if (strcmp(g_pcTmpBuf, "/")) {
                strcat(g_pcTmpBuf, "/");
            }

            //
            // Append the new directory to the path.
            //
            strcat(g_pcTmpBuf, argv[1]);
        }
    }

    //
    // At this point, a candidate new directory path is in chTmpBuf.  Try to
    // open it to make sure it is valid.
    //
    iFResult = f_opendir(&g_sDirObject, g_pcTmpBuf);

    //
    // If it can't be opened, then it is a bad path.  Inform the user and
    // return.
    //
    if (iFResult != FR_OK) {
        printf("cd: %s\r\n", g_pcTmpBuf);
        return ((int) iFResult);
    }

    //
    // Otherwise, it is a valid new path, so copy it into the CWD.
    //
    else {
        strncpy(g_pcCwdBuf, g_pcTmpBuf, sizeof(g_pcCwdBuf));
    }

    //
    // Return success.
    //
    return (0);
}

//*****************************************************************************
//
// This function implements the "pwd" command.  It simply prints the current
// working directory.
//
//*****************************************************************************
int Cmd_pwd(int argc, char *argv[]) {

    printf("\r\n");

    //
    // Print the CWD to the console.
    //
    printf("%s\r\n", g_pcCwdBuf);

    //
    // Return success.
    //
    return (0);
}

//*****************************************************************************
//
// This function implements the "cat" command.  It reads the contents of a file
// and prints it to the console.  This should only be used on text files.  If
// it is used on a binary file, then a bunch of garbage is likely to printed on
// the console.
//
//*****************************************************************************
int Cmd_cat(int argc, char *argv[]) {
    FRESULT iFResult;
    uint32_t ui32BytesRead;

    //
    // First, check to make sure that the current path (CWD), plus the file
    // name, plus a separator and trailing null, will all fit in the temporary
    // buffer that will be used to hold the file name.  The file name must be
    // fully specified, with path, to FatFs.
    //
    if (strlen(g_pcCwdBuf) + strlen(argv[1]) + 1 + 1 > sizeof(g_pcTmpBuf)) {
        printf("Resulting path name is too long\r\n");
        return (0);
    }

    //
    // Copy the current path to the temporary buffer so it can be manipulated.
    //
    strcpy(g_pcTmpBuf, g_pcCwdBuf);

    //
    // If not already at the root level, then append a separator.
    //
    if (strcmp("/", g_pcCwdBuf)) {
        strcat(g_pcTmpBuf, "/");
    }

    //
    // Now finally, append the file name to result in a fully specified file.
    //
    strcat(g_pcTmpBuf, argv[1]);

    //
    // Open the file for reading.
    //
    iFResult = f_open(&g_sFileObject, g_pcTmpBuf, FA_READ);

    //
    // If there was some problem opening the file, then return an error.
    //
    if (iFResult != FR_OK) {
        return ((int) iFResult);
    }

    printf("\r\n");

    //
    // Enter a loop to repeatedly read data from the file and display it, until
    // the end of the file is reached.
    //
    do {
        //
        // Read a block of data from the file.  Read as much as can fit in the
        // temporary buffer, including a space for the trailing null.
        //
        iFResult = f_read(&g_sFileObject, g_pcTmpBuf, sizeof(g_pcTmpBuf) - 1,
                (UINT *) &ui32BytesRead);

        //
        // If there was an error reading, then print a newline and return the
        // error to the user.
        //
        if (iFResult != FR_OK) {
            printf("\r\n");
            return ((int) iFResult);
        }

        //
        // Null terminate the last block that was read to make it a null
        // terminated string that can be used with printf.
        //
        g_pcTmpBuf[ui32BytesRead] = 0;

        //
        // Print the last chunk of the file that was received.
        //
        printf("%s", g_pcTmpBuf);
    } while (ui32BytesRead == sizeof(g_pcTmpBuf) - 1);

    printf("\r\n");
    //
    // Return success.
    //
    return (0);
}

int fputc(int _c, register FILE *_fp)
{
    while(!(UCA0IFG&UCTXIFG));
    UCA0TXBUF = (unsigned char)_c;
    return ((unsigned char)_c);
}

int fputs(const char *_ptr, register FILE *_fp)
{
    unsigned int i, len;
    len = strlen(_ptr);

    for(i = 0; i < len; i++)
    {
        while(!(UCA0IFG&UCTXIFG));
        UCA0TXBUF = (unsigned char) _ptr[i];
    }

    return len;
}


#endif /* SDCARD_H_ */
