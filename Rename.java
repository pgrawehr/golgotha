import java.io.*; 

class Rename { 

static void DoRename(File dir) 
{ 
File fileList[] = dir.listFiles(); 
for (int i = 0; i < fileList.length; ++i) 
{ 
File f = fileList[i]; 
if (f.isDirectory()) 
{ 
DoRename(f); 
} 
String name = f.getName(); 
if (name.endsWith(".cc")) 
{ 
String newName = name.substring(0, name.length() - 3) + ".cpp"; 
File newFile = new File(dir, newName); 
System.out.println("Rename " + name + " to " + 
newFile.getAbsolutePath()); 
f.renameTo(newFile); 
} 
else if (name.endsWith(".hh")) 
{ 
String newName = name.substring(0, name.length() - 3) + ".h"; 
File newFile = new File(dir, newName); 
System.out.println("Rename " + name + " to " + 
newFile.getAbsolutePath()); 
f.renameTo(newFile); 
} 
} 
} 

static void main(String args[]) 
{ 
File file = new File("."); 
DoRename(file); 
} 

} 
