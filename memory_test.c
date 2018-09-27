/*
 ============================================================================
 Name        : memory_test.c
 Author      :
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : ZedBoard FPGA Memory Test in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "memory_api.h"

/*
 **************** Macros and Constants definition ***************
 */

#define OPTION_EXIT         0x1B  // ESC
#define OPTION_TEST_01      0x31  // 1
#define OPTION_TEST_02      0x32  // 2
#define OPTION_TEST_03      0x33  // 3
#define OPTION_TEST_04      0x34  // 4
#define OPTION_TOGGLE_FAIL  0x35 //5
#define OPTION_LFSR_TEST    0x36 //6

// BRAM
#define BRAM_PORT_A_BASE_ADDRESS  0x40000000
#define BRAM_PORT_B_BASE_ADDRESS  0x40002000

// GPIO
#define GPIO_LED_ADDRESS        0x43C10004
#define GPIO_BUTTONS_ADDRESS    0x41200000

// LSFR
#define LFSR_32BIT_ADDRESS      0x43C00000
#define LFSR_10BIT_ADDRESS      0x43C00004

#define clear() printf("\033[H\033[J")

/*
 **************** Function Prototypes ***************
 */

int
memory_test_01 (void);

int
memory_test_02 (void);

int
memory_test_03 (void);

int
memory_test_all (void);

int
menu (void);

int
generic_memory_test (unsigned long port_write, unsigned long port_read);

void
insert_failure (unsigned int* data1);

int FORCE_FAIL = 0;
int memoryArr[1024];

/*
 **************** Function Implementation ***************
 */

int
menu (void)
{
  int option = OPTION_EXIT;
  int res = 0;
  int always_one = 1;

  clear();
  printf ("\n\n\nZedBoard FPGA Memory Test\n");
  printf ("\nPseudo random address and data test of the BRAM memory\n");
  printf ("Select a test option and press ENTER:\n");
  printf ("1. Test using Port A.\n");
  printf ("2. Writes data to Port-A and reads the results from Port-B.\n");
  printf ("3. Writes data to Port-B and reads the results from Port-A.\n");
  printf ("4. Test All Forever.\n");
  printf ("5. Toggle Fail.\n");
  printf ("Press ESC to exit.\n");

  restart: option = OPTION_TEST_04;//getchar ();

  switch (option)
    {
    case OPTION_TEST_01:
      {
        res = memory_test_01 ();
        break;
      }
    case OPTION_TEST_02:
      {
        res = memory_test_02 ();
        break;
      }
    case OPTION_TEST_03:
      {
        res = memory_test_03 ();
        break;
      }
    case OPTION_TEST_04:
      {
	//while(1) {
        res = memory_test_all ();
        //pm (LFSR_10BIT_ADDRESS, &always_one);
	//}
        break;
      }
    case OPTION_EXIT:
      {
        int led_val = 0;
        pm (GPIO_LED_ADDRESS, &led_val);
        exit (0);
      }
    case OPTION_TOGGLE_FAIL:
      {
        FORCE_FAIL = !FORCE_FAIL;
        if (FORCE_FAIL)
        {
          printf ("FAIL MODE ACTIVATED\n");
        }
        else
        {
          printf ("PASS MODE ACTIVATED\n");
        }
        goto restart;
        break;
      }
    case OPTION_LFSR_TEST:
      {

        int i, data;
        int array[1024];

        // initialize array to 0
        for (i = 0; i < 1024; i++)
        {
          array[i] = 0;
        }

        // insert values into array based on LFSR values
        array[0] = 1;
        for (i = 1; i < 1024; i++)
        {
          dm (LFSR_10BIT_ADDRESS, &data);
          array[data] = 1;
        }

        // test values
        for (i = 0; i < 1024; i++)
        {
          if (!array[i])
          {
            printf ("10 Bit LFSR failed for address 0x%x\n", i);
            goto restart;
          }
        }
        printf ("10 Bit LFSR passed\n");
        break;

      }
    default:
      {
        break;
      }

    }
  if (res)
  {
    printf ("Test Failed\n\n\n");
    int led_val = 255;
    while (1)
    {
      led_val = 255;
      pm (GPIO_LED_ADDRESS, &led_val);
      sleep (1);
      led_val = 0;
      pm (GPIO_LED_ADDRESS, &led_val);
      sleep (1);
    }
    exit (-1);
  }
  else
  {
    printf ("Test Passed\n\n\n");
    int led_val = 255;
    pm (GPIO_LED_ADDRESS, &led_val);
  }
  return option;
}

int
generic_memory_test (unsigned long port_write, unsigned long port_read)
{
  int data1 = 0, data2 = 0;
  int result = 0;
  int i = 0;
  int address = 0;

  // initialize memory array
  for (i = 0; i < 1024; i++)
  {
    memoryArr[i] = 0;
  }
  printf ("STARTING PART A\n");
  // Call dm function
  dm (LFSR_32BIT_ADDRESS, &data1);
  pm (port_write + 0, &data1);
  memoryArr[0] = data1;
  printf ("address = 0x%lx, data1 = 0x%x\n", port_write, data1);
  printf ("Write all A.\n");
  for (i = 1; i < 1024; ++i)
  {
    int led_val = (1 << i / 32) % 256;
    pm (GPIO_LED_ADDRESS, &led_val);
    dm (LFSR_32BIT_ADDRESS, &data1);
    dm (LFSR_10BIT_ADDRESS, &address);
    pm (port_write + (address << 2), &data1);
    memoryArr[address] = data1;
    printf ("i: %d, address = 0x%08lx, data1 = 0x%x\n", i,
            port_write + (address << 2), data1);
  }
  printf ("Read all A\n");
  for (i = 0; i < 1024; ++i)
  {
    int led_val = (1 << i / 32) % 256;
    dm (LFSR_10BIT_ADDRESS, &address);
    dm (port_read + (address << 2), &data2);
    if (FORCE_FAIL)
    {
      insert_failure (&data2);
    }
    printf ("i: %d, address = 0x%08lx, data2 = 0x%x\n", i,
            port_read + (address << 2), data2);
    if (memoryArr[address] != data2)
    {
      result = -1;
      return result;
    }
  }

  // initialize memory array
  for (i = 0; i < 1024; i++)
  {
    memoryArr[i] = 0;
  }
  printf ("\nSTARTING PART B\n");
  // Call dm function
  dm (LFSR_32BIT_ADDRESS, &data1);
  pm (port_write + 0, &data1);
  memoryArr[0] = data1;
  printf ("address = 0x%08lx, data1 = 0x%x\n", port_write, data1);
  printf ("Write all B.\n");
  for (i = 1; i < 1024; ++i)
  {
    int led_val = (1 << i / 32) % 256;
    pm (GPIO_LED_ADDRESS, &led_val);
    dm (LFSR_32BIT_ADDRESS, &data1);
    dm (LFSR_10BIT_ADDRESS, &address);
    pm (port_write + (address << 2), &data1);
    memoryArr[address] = data1;
    printf ("i: %d, address = 0x%08lx, data1 = 0x%x\n", i,
            port_write + (address << 2), data1);
  }
  unsigned int sleepTime = ((data1 & 0x00000FFF) % 10) + 1;
  printf ("\nsleeping for %d\n", sleepTime);
  sleep (sleepTime);
  printf ("Read all B\n");
  for (i = 0; i < 1024; ++i)
  {
    int led_val = (1 << i / 32) % 256;
    dm (LFSR_10BIT_ADDRESS, &address);
    dm (port_read + (address << 2), &data2);
    if (FORCE_FAIL)
    {
      insert_failure (&data2);
    }
    printf ("i: %d, address = 0x%08lx, data2 = 0x%x\n", i,
            port_read + (address << 2), data2);
    //   data2 = ~data2;
    if (memoryArr[address] != data2)
    {
      result = -1;
      break;
    }
  }

  return result;
}

int
memory_test_01 (void)
{
  int result = generic_memory_test (BRAM_PORT_A_BASE_ADDRESS,
  BRAM_PORT_A_BASE_ADDRESS);

  return result;
}

int
memory_test_02 (void)
{
  int result = generic_memory_test (BRAM_PORT_A_BASE_ADDRESS,
  BRAM_PORT_B_BASE_ADDRESS);

  return result;
}

int
memory_test_03 (void)
{
  int result = generic_memory_test (BRAM_PORT_B_BASE_ADDRESS,
  BRAM_PORT_A_BASE_ADDRESS);

  return result;
}

int
memory_test_all (void)
{
  int result = 0;

  result = memory_test_01 ();
  result |= memory_test_02 ();
  result |= memory_test_03 ();
}

void
insert_failure (unsigned int* data1)
{
  *data1 = 0xdeadbeef;
}

/*
 **************** Main Function ***************
 */

int
main (void)
{
  int option = OPTION_EXIT;
  do
  {
    option = menu ();

  }
  while (option != OPTION_EXIT);

  return EXIT_SUCCESS;
}
