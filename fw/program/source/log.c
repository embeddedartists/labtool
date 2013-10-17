/*!
 * @file
 * @brief   Log functionality to aid debugging
 *
 * @copyright Copyright 2013 Embedded Artists AB
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


/******************************************************************************
 * Includes
 *****************************************************************************/

#include <string.h>

#include "log.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

/******************************************************************************
 * Global variables
 *****************************************************************************/

/******************************************************************************
 * Global Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/


/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief   Prints data in a in a readable form.
 *
 * The data will be prepended with an offset and append a ascii representation.
 * Example output:
 *
 *     00000000  48 65 6c 6c 6f 20 57 6f 72 6c 64 21 21 0d 0a     Hello World!!..
 *
 * @param[in] buffer  The data to print
 * @param[in] size    Number of bytes of the data to print
 *
 *****************************************************************************/
void display_buffer_hex(const unsigned char * const buffer, unsigned size)
{
  unsigned i, j, k;
  char buff[128];
  int off;
  for (i=0; i<size; i+=16)
  {
    off = 0;
    off += sprintf(buff+off, "  %08x  ", i);
    for(j=0,k=0; k<16; j++,k++)
    {
      if (i+j < size)
      {
        off += sprintf(buff+off, "%02x", buffer[i+j]);
      }
      else
      {
        off += sprintf(buff+off, "  ");
      }
      off += sprintf(buff+off, " ");
    }
    off += sprintf(buff+off, " ");
    for(j=0,k=0; k<16; j++,k++)
    {
      if (i+j < size)
      {
        if ((buffer[i+j] < 32) || (buffer[i+j] > 126))
        {
          off += sprintf(buff+off, ".");
        }
        else
        {
          off += sprintf(buff+off, "%c", buffer[i+j]);
        }
      }
    }
    _DBG_(buff);
  }
}

