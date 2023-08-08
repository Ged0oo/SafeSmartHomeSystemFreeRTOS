# SafeSmartHomeSystemFreeRTOS
FreeRTOS smart home demo project,

# In this demo we have 2 semaphores to synchronize the access of Keypad and LCD through 4 main tasks.

# First task is the intialization task and this task just intializes the application and the lcd, keypad and wait until the user enters the right password Rather than locking the system in case of consume its three tries then this task will delete it self.

# The second task is the keypad task and it's higher priority than the LCD task , so the scheduler will execute this task after the intialization task and it's function is very simple just gets the input from the keypad then it will block it self.

# The third task is the LCD frames it just puts the required frame on the lcd after applying the up or down direction, and if the input was to select this mood , this task will block it self to let the scheduler execute the last task.

# The last task is to get the information of the required mood and returns to the home mood to scroll again and again.

# The synchronization is done by two semaphores, one is to synchronize the access of keypad and lcd so when the keypad task executes it will blocked it self by this semaphore.

# The second semaphore is to synchronize the access of the system frames.