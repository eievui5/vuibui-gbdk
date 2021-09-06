Cutscenes in Vuiiger are programmed using a Macro-based scripting system.

C macros are fairly limited compared to RGBDS macros or a proper script compiler, so there are a few workarounds that should be kept in mind:
- C Macros don't have the same label support as RGBDS, so branching must be done by defining a new script and jumping to it.
- A (possible?) bug with SDCC means that strings cannot be defined *within* scripts. Instead, create a `const char string[]` and pass it to the scripting macros.

# Commands:

### `ANIMATE_ENTITY:`
- Oscillate between two given cels every few frames. Automatically renders new cels.

### `JUMP:`
- Jump to a script.

### `JUMP_IF_TRUE:`
- Jump to a script if the last conditional command returned a non-zero value.

### `LOAD_ENTITY:`
- Loads a new entity to a given index, allowing them to be controlled.

### `MOVE_ENTITY:`
- Move an entity towards a given X *or* Y location. Mixing both will look strange and should be avoided.
- `Returns = `True if the location was reached, false otherwise.

### `RENDER_ENTITY:`
- Reloads a given entity's graphics. Use after a frame or direction has been updated.

### `SAY:`
- Enter dialogue mode and display a string.

### `SET:`
- Set a pointer to a given constant 8-bit value.

### `SET_ENTITY_DIR:`
### `SET_ENTITY_FRAME:`
### `SET_ENTITY_POS:`
- Set the members of a given entity index.

### `YIELD:`
- End script processing until the next frame. Effectively waits one frame.

# Dialogue:

Dialogue uses certain control characters to interface with the dialogue and scripting systems. Control characters are prefaced with a `%`, like C format characters. This is mostly for syntax highlighting, and because dialogue is unlikely to ever use a "%".

### `%c` - clear
- Clear the dialogue box.

### `%q` - question
- Ask a question. Will jump to the next line and write text until hitting the end of the screen, returning true to the script if the *second* option was chosen, and false for the first.

Example:
```
"Yes or No?%qYes.\nNo."
```
Produces:
```
Yes or No?
 > Yes.
   No.
```

### `%w` - wait
- Wait for the player to press A.