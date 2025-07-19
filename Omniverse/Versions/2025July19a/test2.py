from omni.kit_app import KitApp


app = KitApp()


app.shutdown()

exit()
#import numpy as np 


import carb

carb.log_info("Hello from Kit script!")


import omni.kit.app

omni.kit.app.get_app().post_quit()



print("✅ Script executed successfully inside Omniverse Kit.")