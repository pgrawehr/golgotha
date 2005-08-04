import java.io.*; 

class backup { 
static void BackupFile(File src,String Name) 
{ 
        String fullPath = src.getAbsolutePath(); 
        System.out.println("Copying " + fullPath); 
        try 
        { 
                File dst = File.createFile(Name, null,
                        "d:\\golgotha\\backup\\"); 
                BufferedWriter out = new BufferedWriter(new FileWriter(dst)); 
                try 
                { 
                        BufferedReader in = new BufferedReader(new FileReader(src)); 
                        while (true) 
                        { 
                                String line = in.readLine(); 
                                if (line == null) 
                                break; 
                                out.write(line, 0, line.length()); 
                                out.newLine(); 
                        } 
                        in.close(); 
                } 
                catch (FileNotFoundException e) 
                { 
                        return; 
                } 
                catch (IOException e) 
                { 
                // remember to delete temp file 
                return; 
                } 
                out.close(); 
        } 
        catch (IOException e) 
        { 
        // temp file create failed 
        } 
} 

static void BackupDir(File dir) 
{ 
        File fileList[] = dir.listFiles(); 
        for (int i = 0; i < fileList.length; ++i) 
        { 
                File f = fileList[i]; 
                if (f.isDirectory()) 
                { 
                        BackupDir(f); 
                } 
                else 
                { 
                        String name = f.getName(); 
                        if (name.endsWith(".cpp")) 
                        { 
                                BackupFile(f,name); 
                        } 
                        else if (name.endsWith(".h")) 
                        { 
                                BackupFile(f,name); 
                        }
                        else if(name.endsWith(".scm"))
                        {
                            BackupFile(f,name);
                        }
                        else if(name.endsWith(".res"))
                        {
                            BackupFile(f,name);
                        }
                } 
        } 
} 

static void main(String args[]) 
{
    System.out.println("Starting backup operation");
    BackupDir(new File("."));
    System.out.println("Backup completed");

} 
} 
