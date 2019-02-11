/*
 * Copyright (C) 2018 dimercur
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <unistd.h>

#include <sys/mman.h>

#ifdef __WITH_PTHREAD__
#include "tasks_pthread.h"
#else
#include "tasks.h"
#endif // __WITH_PTHREAD__

int main(int argc, char **argv) {
    Tasks tasks;
    
    //Lock the memory to avoid memory swapping for this program
    mlockall(MCL_CURRENT | MCL_FUTURE);

    cout<<"#################################"<<endl;
    cout<<"#      DE STIJL PROJECT         #"<<endl;
    cout<<"#################################"<<endl;

    tasks.Init();
    tasks.Run();
    tasks.Join();
    
    tasks.Stop();
    
    //tasks.Run();

    return 0;
}

