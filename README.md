# RUDP BASED CHAT BOX WRITTEN IN PURE C
- Note this works just in LINUX/UNIX-LIKE SYSTEMS(FREEBSD...)

## Screenshot:
- OLD ONE I DONT HAVE TIME FOR THE NEW RUDP BASE
![ScreenShot](assets/screenshot.png)

## Time:
- Started       01/11/2023 11:00 PM // As I do the Actual imple and the work of yesterday gone
- Finished      02/11/2023 23:11 PM (still writing this READMI)

## Features:
- No dependencies.
- MuliClients (Up to 32 client!).
- Plug and Play.
- Enjot it.

## Note:
- I decided to make a Game then I see that enven the server is over kill so i stop.
- I did not use the heap, as I do not want to debug segmentaion faults
- I use simple model

    ```
                         ----------------
                        |                |
                        |                |
             Chat ======|      RUDP      |====== Server/Client UDP communication
                        |                |
                        |                |
                         ----------------
    ```
- Note as I thout a lot about the implementation I add some comments that doas not make a sense

## Known issues:
-No known issues.

## Install / Run:
- run `./build.sh` and every thing will be fine.


## Licence:
- BIGMAMA/DxUr I dont know this is not a program this is just a test