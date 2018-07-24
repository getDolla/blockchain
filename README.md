# Blockchain
for Revo-Gene  

To Compile, use Qt Creator  

[中文翻译，请点击这里](#中文翻译)

## FAQ:  
- #### What version do I use?  
> English version: Use the version marked **EN**  
> 中文版: 用标 **CN** 的版本  
> Users with the English version can connect to users with the Chinese version and vice versa.

- #### How do I use this?  
> On Windows, double-click `Blockchain.exe`, **NOT `blockchain`** to run the application  

   * ##### Interface  
   ![interface_en](/images/main_en.jpg "Main Interface")  
      * ##### Connect to Another PC  
      > When clicked, user will be prompted to enter the other computer's IP address and port number. Being connected to multiple computers greatly reduces the chance of data corruption and ensures that the blockchain is up to date.  

      * ##### Store file to Blockchain  
      > When clicked, user will be prompted to select a file from their computer to be appended (added) to the blockchain (increases the blockchain's length by 1). The hash will be displayed after the file has been added to a block and the block appended (mined) to the chain.  

      * ##### Save Block to PC + List All Files (See Below for Usage)  
      * ##### Update Blockchain  
      > When clicked, the blockchain application will check connected computers for any changes made and updates the blockchain on the user's computer. The blockchain will be saved in `blockchain`.  

   * ##### Saving a file to PC
   ![save_one_en](/images/save_en.jpg "Listing All Files")  
   > First, click on `List All Files` to get the list of files currently on the blockchain and their respective index  

   ![save_two_en](/images/file_en.jpg "Example Save")
   > Next, change the index to the number corresponding to the file you want to save on the computer.  
   > Finally, click `Save Block to PC` to select the location of where you want to save the file  

   * ##### Auto-Connect Feature  
   > The blockchain app will automatically save all current connections into `connections.txt` when the application closes.  
   > If you wish to have the app automatically connect to other computers upon start-up, simply edit `connections.txt`  
   > `connections.txt` keeps IP addresses and port numbers of previously connected computers in the following format:  

   ```
   <IP Address> <Port Number>
   ```
   > The contents of `connections.txt` may look like the example below:  

   ```
   60.25.142.119 41293
   88.232.225.36 53945
   83.95.250.94 59178
   238.47.114.69 61170
   ```
- #### Why should I use this blockchain app instead of cloud-storage?  
>* ##### It is free.  
>* ##### Companies, such as Long Island Drinks Corp, who renamed themselves to include blockchain in their name saw their stock rose dramatically; [Long Island Drinks Corp's stock rose 200% after being renamed to Long Blockchain](https://cointelegraph.com/news/blockshow-announces-blockshow-americas-2018-conference-in-las-vegas-august-20-21 "Link to News Article").  

- #### Help! blockchain.exe is frozen/not responding!
> Just wait (until it crashes). It might be computing the file's hash, sending a large file, or another computer just so happens to upload a file to the blockchain at the same time.  

- #### Help! I can't update my blockchain!  
> Open the **file** `blockchain` with a text editor and paste this:  

`
0  1531877114 TkFNRTogIERBVEE6IEdlbmVzaXMgQmxvY2s= 0 91629c3d6c07c2d358688ef3daedf9a420dbeb7d518b08de4bcfa4af2488f400bc3a9ecfd70697780213210ce172e0720adf7c160fb4b6b4cc54489fb36945b9
`  

<a name="中文翻译">
#   中文翻译
</a>
