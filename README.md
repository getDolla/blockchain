# Blockchain
for 原创基因 (Revo-Gene)  

To Compile, use Qt Creator  

[中文翻译，请点击这里](#中文翻译)

## FAQ:  
- **What version do I use?**  
English version: Use the version marked **EN**  
中文版: 用标 **CN** 的版本  
Users with the English version can connect to users with the Chinese version and vice versa.  

- **How do I use this?**  
On Windows, double-click `Blockchain.exe`, **NOT `blockchain`**, to run the application.  

- **Why should I use this blockchain app instead of cloud-storage?**  

   ##### It is free.  
   ##### Companies, such as Long Island Drinks Corp, who renamed themselves to include blockchain in their name saw their stock rose dramatically; [Long Island Drinks Corp's stock rose 200% after being renamed to Long Blockchain](https://cointelegraph.com/news/blockshow-announces-blockshow-americas-2018-conference-in-las-vegas-august-20-21 "Link to News Article").  

- **Help! blockchain.exe is frozen/not responding!**
Just wait (until it crashes). It might be computing the file's hash, sending a large file, or another computer just so happens to upload a file to the blockchain at the same time.  

- **Help! I can't update my blockchain!**  
Open the **file** `blockchain` with a text editor, delete all text, then paste this:  

`
0  1531877114 TkFNRTogIERBVEE6IEdlbmVzaXMgQmxvY2s= 0 91629c3d6c07c2d358688ef3daedf9a420dbeb7d518b08de4bcfa4af2488f400bc3a9ecfd70697780213210ce172e0720adf7c160fb4b6b4cc54489fb36945b9
`  

## Interface  
   ![interface_en](/images/main_en.jpg "Main Interface")  
##### Connect to Another PC  

When clicked, user will be prompted to enter the other computer's IP address and port number. Being connected to multiple computers greatly reduces the chance of data corruption and ensures that the blockchain is up to date.  

##### Store file to Blockchain  

When clicked, user will be prompted to select a file from their computer to be appended (added) to the blockchain (increases the blockchain's length by 1). The block's hash will be displayed after the file has been added to a block and the block has been appended (mined) to the chain.  

##### Save Block to PC + List All Files (See Below for Usage)  

##### Update Blockchain  

When clicked, the blockchain application will check connected computers for any changes made and updates the blockchain on the user's computer. The blockchain will be saved in `blockchain`.  

### Saving a file to PC  
   ![save_one_en](/images/save_en.jpg "Listing All Files")  
   First, click on `List All Files` to get the list of files currently on the blockchain and their respective index.  

   ![save_two_en](/images/file_en.jpg "Example Save")  
   Next, change the index to the number corresponding to the file you want to save on the computer.  
   Finally, click `Save Block to PC` to select the location of where you want to save the file.  

### Auto-Connect Feature  
   The blockchain app will automatically save all current connections into `connections.txt` when the application closes.  
   If you wish to have the app automatically connect to other computers upon start-up, simply edit `connections.txt`.  
   `connections.txt` keeps IP addresses and port numbers of previously connected computers in the following format:  

   ```
   <IP Address> <Port Number>
   ```
   The contents of `connections.txt` may look like the example below:  

   ```
   60.25.142.119 41293
   88.232.225.36 53945
   83.95.250.94 59178
   238.47.114.69 61170
   ```  


<a name="中文翻译"> </a>  
# 中文翻译  
## 常问问题:  
- **我用的是什么版本?**  
中文版: 用标 **CN** 的版本  
English version: Use the version marked **EN**  
具有英文版本的用户可以使用中文版本连接到用户，反之亦然。  

- **我怎么用?**  
在Windows上，双击 `Blockchain.exe`，**不要点击 `blockchain` **，以运行程序。  

- **为什么我应该使用这个区块链应用而不是云存储?**  
   ##### 是免费的。   
   ##### 公司，例如长岛饮料公司（Long Island Drinks Corp），改名为自己的名称包括区块链后看到自己的股票大幅上涨; [Long Island Drinks Corp的股票在更名为Long Blockchain后股价上涨200％](https://cointelegraph.com/news/blockshow-announces-blockshow-americas-2018-conference-in-las-vegas-august-20-21 "新闻文章")。  

- **帮帮我! blockchain.exe 被冻结/没有反应!**  
等一下（直到它崩溃）。它可能是计算文件的哈希值，发送大文件，或其他计算机碰巧同时上传文件到区块链。  

- **帮帮我! 我无法更新我的区块链!**  
用记事本来打开 `blockchain` **文件**，删除所有文字，然后复制粘贴下列文字:  

`
0  1531877114 TkFNRTogIERBVEE6IEdlbmVzaXMgQmxvY2s= 0 91629c3d6c07c2d358688ef3daedf9a420dbeb7d518b08de4bcfa4af2488f400bc3a9ecfd70697780213210ce172e0720adf7c160fb4b6b4cc54489fb36945b9
`  

## 前端  
![interface_cn](/images/main_cn.jpg "前端")  
##### 连接电脑  

点击后，用户会要输入另一台计算机的IP地址和端口号。连接到多台计算机可以大大降低数据损坏的可能性，并确保区块链是最新的。

##### 上传文件  

点击后，用户会要从其计算机中选择要附加（添加）到区块链的文件（区块链的长度会增加一个块）。文件被保存到区块链后，区块的哈希会被显示到。

##### 下载文件到电脑 + 显示文件 （见下面的用法）  

##### 更新数据  
点击后，区块链应用程序会检查连接的计算机是否进行了任何更改，并更新用户计算机上的区块链。 区块链将保存在 `blockchain` 文件上。

### 出区块链保存一个文件到电脑上  
   ![save_one_cn](/images/save_cn.jpg "显示文件")  
   第一步，点击 '显示文件' 来列出所有的文件和它们的各自的指数。  

   ![save_two_cn](/images/file_cn.jpg "示例保存")  
   接下来，把指数更改到在计算机上保存的文件对应的数字。  
   最后，点击 `下载文件到电脑` 以选择要保存文件的位置。
