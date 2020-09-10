import sys
import os
import cv2
import threading

record_file = "record"
record_cmd = None

def start(recorders, filename):
	for (i, c, size, fps, writer) in recorders:
		if writer is None:
			print(f"start recording camera {i} to {filename}_{i}.mp4 ...")
			format = cv2.VideoWriter.fourcc('m', 'p', '4', 'v')
			writer = cv2.VideoWriter(f"{filename}_{i}.mp4", format, fps, size, True)
			recorders[i] = (i, c, size, fps, writer)

def stop(recorders):
	for (i, c, size, fps, writer) in recorders:
		if writer is not None:
			writer.release()
			recorders[i] = (i, c, size, fps, None)
			print(f"recording camera {i} finished")

def record(recorders):
	global record_file
	global record_cmd
	[cv2.namedWindow(f"camera {i}", cv2.WINDOW_NORMAL) for i in range(len(recorders))]
	while True:
		if record_cmd == "start":
			stop(recorders)
			start(recorders, record_file)
			record_cmd = None
		elif record_cmd == "stop":
			stop(recorders)
			record_cmd = None
		elif record_cmd == "exit":
			stop(recorders)
			[cv2.destroyWindow(f"camera {i}") for i in range(len(recorders))]
			return
		for (i, c, size, fps, writer) in recorders:
			ok, img = c.read()
			if ok:
				if writer is not None:
					writer.write(img)
				cv2.imshow(f"camera {i}", img)
				cv2.waitKey(1)
			else:
				print(f"read frame from video capture {i} failed")
				record_cmd = "exit"
				break

def init_recorders():
	recorders = []
	for i in range(8):
		c = cv2.VideoCapture(i)
		if not c.isOpened():
			break
		size = int(c.get(cv2.CAP_PROP_FRAME_WIDTH)), int(c.get(cv2.CAP_PROP_FRAME_HEIGHT))
		fps = c.get(cv2.CAP_PROP_FPS)
		recorders.append((i, c, size, fps, None))
	return recorders

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print(f"Usage: {sys.argv[0]} [out-dir]")
		sys.exit(-1)
	out_dir = sys.argv[1] if len(sys.argv) > 1 else os.curdir
	recorders = init_recorders()
	if len(recorders) == 0:
		print("no video capture devices found")
		sys.exit(-1)
	record_thread = threading.Thread(target=record, args=(recorders,))
	record_thread.start()
	print("input comands: \n  start - start recording\n  stop - stop recording\n  exit - exit this app")
	while True:
		cmds = input().split()
		cmd = cmds[0]
		if cmd == "start" and len(cmds) == 2:
			record_cmd = "start"
			record_file = out_dir + "/" + cmds[1]
		elif cmd == "stop":
			record_cmd = "stop"
		elif cmd == "exit":
			record_cmd = "exit"
			break
		else:
			print(f"invalid command: '{cmd}'")
	record_thread.join()
