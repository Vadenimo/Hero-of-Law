import ctypes, os, sys, shutil
import tkinter as tk
from tkinter import messagebox

APPDATA     = os.environ.get('APPDATA')
BLENDER_300 = os.path.realpath(f'{APPDATA}\\Blender Foundation\\Blender\\3.0\\scripts\\addons')
SRC_PATH    = os.path.split(__file__)[0] + '\\Tool\\io_export_objex2'
DST_PATH    = f'{BLENDER_300}\\io_export_objex2'

def symlink():
	if messagebox.askyesno('Create Symlink?', f'Source folder:\n{SRC_PATH}\n\nDestination folder:\n{DST_PATH}'):
		try:
			os.symlink(SRC_PATH, DST_PATH, target_is_directory=True)
		except OSError as error:
			messagebox.showerror('Error', f'{error}')

def is_admin():
	try:
		return ctypes.windll.shell32.IsUserAnAdmin()
	except:
		return False

if __name__ == "__main__":
	
	if not is_admin():
		ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, " ".join(sys.argv), None, 1)
		exit(0)
	
	root = tk.Tk()
	root.withdraw()
	
	if not os.path.exists(BLENDER_300):
		messagebox.showerror("Error", "Blender 3.0 not found!")
	elif os.path.exists(DST_PATH):
		if DST_PATH == os.path.realpath(DST_PATH): 
			# if paths are identical: symlink
			if messagebox.askyesno('Delete?', 'Found Default Objex2 already installed. Delete?'):
				try:
					shutil.rmtree(DST_PATH)
					symlink()
				except Exception as error:
					messagebox.showerror("Error", f'{error}')
		else:
			messagebox.showinfo("Info", "Already installed!")
	else:
		symlink()
	
	root.destroy()
