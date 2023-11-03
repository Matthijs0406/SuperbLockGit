from encryption import *
from tkinter import *
import customtkinter
from PIL import ImageTk, Image

def get_entry_value():
    entry_text = entry.get()
    return str(entry_text)

window = customtkinter.CTk()
window.geometry("300x400")

frame = customtkinter.CTkFrame(window, width=250, height=200)
frame.pack()

img = ImageTk.PhotoImage(Image.open("logo.png"))

label = Label(frame, image = img)
label.pack(pady = 5)

entry = customtkinter.CTkEntry(master=window, placeholder_text="Enter a UID")

button_add = customtkinter.CTkButton(
    text = "Add or erase this tag",
    command = lambda : modify_uids(encrypt(get_entry_value())),
    master = window
)

entry.pack(pady = 15)
button_add.pack()


window.mainloop()
