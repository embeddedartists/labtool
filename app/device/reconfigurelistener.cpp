/*
 *  Copyright 2013 Embedded Artists AB
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include "reconfigurelistener.h"

/*!
    \class ReconfigureListener
    \brief ReconfigureListener is an interface for reconfiguration events.

    \ingroup Device

    The ReconfigureListener class provides a way to let signals notify a
    device of changes that may require reconfiguration of the device
    without having to expose the entire device interface to the signals.

*/

/*!
    Constructs a reconfiguration listener. This class will never be
    instantiated directly. Instead a subclass will inherit from this
    class.
*/
ReconfigureListener::ReconfigureListener()
{
}

/*!
    \fn virtual void ReconfigureListener::reconfigure(int sampleRate)

    Notifies the implementor of a change that may require a reconfiguration.
*/
