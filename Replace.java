import java.io.*; 

class Replace { 
static void ReplaceFile(File src) 
{ 
String fullPath = src.getAbsolutePath(); 
System.out.println("Replacing in " + fullPath); 
try 
{ 
File dst = File.createTempFile("rpl", null, src.getParentFile()); 
BufferedWriter out = new BufferedWriter(new FileWriter(dst)); 
try 
{ 
BufferedReader in = new BufferedReader(new FileReader(src)); 
while (true) 
{ 
String line = in.readLine(); 
if (line == null) 
break; 
if (line.startsWith("#include")) 
{ 
int offset = line.indexOf(".hh"); 
if (offset >= 0) 
{ 
String prefix = line.substring(0, offset); 
String suffix = line.substring(offset + 3); 
line = prefix + ".h" + suffix; 
System.out.println(line); 
} 
} 
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
src.delete(); 
dst.renameTo(new File(fullPath)); 
} 
catch (IOException e) 
{ 
// temp file create failed 
} 
} 

static void ReplaceDir(File dir) 
{ 
File fileList[] = dir.listFiles(); 
for (int i = 0; i < fileList.length; ++i) 
{ 
File f = fileList[i]; 
if (f.isDirectory()) 
{ 
ReplaceDir(f); 
} 
else 
{ 
String name = f.getName(); 
if (name.endsWith(".cpp")) 
{ 
ReplaceFile(f); 
} 
else if (name.endsWith(".h")) 
{ 
ReplaceFile(f); 
} 
} 
} 
} 

static void main(String args[]) 
{ 
ReplaceDir(new File(".")); 
} 
} 
