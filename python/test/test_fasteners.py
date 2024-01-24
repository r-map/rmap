import fasteners
import time

rw_lock = fasteners.InterProcessReaderWriterLock('/tmp/lock.file')  # for processes

print ("I want write")
with rw_lock.write_lock():
    # write access
    print("Start write")
    time.sleep(10)
    print("End write")

time.sleep(3)
    
# or alternatively
print ("I want write 2")
rw_lock.acquire_write_lock()
# write access
print("Start write 2")
time.sleep(10)
print("End write 2")
rw_lock.release_write_lock()

