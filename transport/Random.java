// Simple class to get list of random numbers
// Caches response.
// @author Scott Carlson <ScottCarlson@excite.com>
// 
import java.net.*;
import java.io.*;
import java.util.*;

public class Random
{
   private int minNum = 0;
   private int maxNum = 100;
   private int cacheSize = 100;
   private Vector cachedList = new Vector();
   private Iterator i = null;
   private Random() {};
   private Random(int min, int max) 
   { minNum = min; maxNum = max; };
   private Random(int size, int min, int max) 
   { cacheSize = size;minNum = min; maxNum = max; };
   public int next()
   {
      if (i == null || !i.hasNext())
      {
           cachedList = getNewList(cacheSize, minNum, maxNum);
           i= cachedList.iterator();
      }
      return Integer.parseInt((String)i.next());
   }
   private Vector getNewList(int size, int min, int max)
   {
      Vector returnList = new Vector();
      try { 
         URL randomOrg = new URL("http://www.random.org/cgi-bin/randnum?"+
                                 "num="+size+"&min="+min+"&max="+max+"&col=1");
         HttpURLConnection con= (HttpURLConnection) randomOrg.openConnection();
         con.setRequestMethod("GET");
         con.setDoInput(true);
         InputStream in = con.getInputStream();
         InputStreamReader isr = new InputStreamReader(in);
         BufferedReader br = new BufferedReader(isr);
         String theLine;
         while ((theLine = br.readLine()) != null) {
             returnList.add(theLine);
         }
         con.disconnect();
      } 
      catch (Exception e)
      {
      }
      return returnList;
   }
   public final static void main(String[] args)
   {
      Random rand = new Random(0, 10);
      for (int i =0; i< 1000; i++)
      {
         System.out.print(" "+rand.next());
      }
   }
}
