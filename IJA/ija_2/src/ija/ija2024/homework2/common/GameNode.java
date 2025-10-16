package ija.ija2024.homework2.common;
import ija.ija2024.tool.common.AbstractObservableField;

import java.util.HashSet;
import java.util.Set;

public abstract class GameNode extends AbstractObservableField {
    protected Position position;
    protected boolean lit = false;
    protected Set<Side> connectors = new HashSet<>();

    public abstract boolean isBulb();
    public abstract boolean isLink();
    public abstract boolean isPower();
    public abstract void turn();
    public abstract String toString();


    public GameNode(Position position) {
        this.position = position;
    }

    public Position getPosition() {
        return this.position;
    }

    public boolean containsConnector(Side s){
        return connectors.contains(s);
    }
    public Set<Side> getConnectors(){
        return new HashSet<>(connectors);
    }



    @Override
    public boolean north() { return containsConnector(Side.NORTH); }

    @Override
    public boolean east() { return containsConnector(Side.EAST); }

    @Override
    public boolean south() { return containsConnector(Side.SOUTH); }

    @Override
    public boolean west() { return containsConnector(Side.WEST); }

    public boolean light(){
        return lit;
    }

    public void setLit(boolean lit){
        if(this.lit != lit){
            this.lit = lit;
            notifyObservers();
        }
    }
}