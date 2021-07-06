class Solution {
public:
    int countNegatives(vector<vector<int>>& grid) {
        int count = 0;
        int c = grid.size();
        int r = grid[0].size();
        
        int i = 0;
        int j = grid[0].size()-1;
        
        while(i < c && j >= 0)
        {
            if(grid[i][j] < 0)
            {
                count += c-i;
                j--;
            }
            else
            {
                i++;
            }
        }
        return count;
    }
};

